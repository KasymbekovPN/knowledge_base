[[raw data/cpp/interview/_|<=]]


# Мьютексы, RAII-локи, deadlock

## Зачем: data race

**Data race** — два потока обращаются к одной памяти, хотя бы один пишет, без синхронизации → **UB** (не «неверное значение», а именно UB — компилятор вправе на этом оптимизировать).

```cpp
int counter = 0;

void increment() {
    for (int i = 0; i < 100000; ++i)
        ++counter;   // ⚠️ DATA RACE — не атомарно: load, +1, store
}

std::thread t1(increment), t2(increment);
t1.join(); t2.join();
std::cout << counter;   // НЕ 200000 — потерянные обновления
```

---

## Типы мьютексов

```cpp
std::mutex                      // базовый
std::recursive_mutex            // один поток может лочить повторно (счётчик)
std::timed_mutex                // + try_lock_for / try_lock_until
std::recursive_timed_mutex
std::shared_mutex               // C++17: reader-writer lock
std::shared_timed_mutex         // C++14
```

### `std::mutex` — базовый

```cpp
std::mutex mtx;

mtx.lock();      // блокируется, пока не захватит
// критическая секция
mtx.unlock();

mtx.try_lock();  // → bool, не блокируется
```

**Никогда не используй `lock()`/`unlock()` вручную** — при исключении или раннем `return` мьютекс останется залоченным → deadlock:

```cpp
void bad() {
    mtx.lock();
    mayThrow();     // ⚠️ исключение → unlock() не вызовется → мьютекс навсегда залочен
    mtx.unlock();
}
```

Отсюда — RAII-обёртки.

**Важно:** повторный `lock()` тем же потоком → **UB** (обычно deadlock):

```cpp
mtx.lock();
mtx.lock();   // ⚠️ UB! std::mutex НЕ рекурсивен
```

### `std::recursive_mutex`

Один поток может захватить многократно (ведётся счётчик; освобождается после стольких же `unlock`).

```cpp
std::recursive_mutex rmtx;

void a() { std::lock_guard lock(rmtx); b(); }   // ✅ работает
void b() { std::lock_guard lock(rmtx); /* ... */ }
```

**Обычно — запах кода:** необходимость рекурсивного мьютекса чаще означает, что публичный метод зовёт другой публичный метод, оба лочатся. Правильнее вынести приватный метод **без** локов:

```cpp
class Widget {
    mutable std::mutex mtx_;
    void doWorkImpl() { /* БЕЗ лока — вызывается уже под локом */ }
public:
    void a() { std::lock_guard lock(mtx_); doWorkImpl(); b_impl(); }
    void b() { std::lock_guard lock(mtx_); b_impl(); }
private:
    void b_impl() { /* без лока */ }
};
```

Плюс `recursive_mutex` медленнее (счётчик, id владельца).

### `std::shared_mutex` (C++17) — reader-writer

Много читателей **или** один писатель.

```cpp
class Cache {
    std::map<int, std::string> data_;
    mutable std::shared_mutex mtx_;   // mutable — лочим в const-методе (мы это разбирали)

public:
    std::string read(int key) const {
        std::shared_lock lock(mtx_);        // ✅ РАЗДЕЛЯЕМАЯ блокировка — много читателей сразу
        auto it = data_.find(key);
        return it != data_.end() ? it->second : "";
    }

    void write(int key, std::string v) {
        std::unique_lock lock(mtx_);        // ✅ ЭКСКЛЮЗИВНАЯ — только один писатель
        data_[key] = std::move(v);
    }
};
```

Выгодно, когда **чтений намного больше записей**. При частых записях — оверхед (shared_mutex дороже обычного) может перевесить.

---

## RAII-обёртки

### `std::lock_guard` — простейший

```cpp
std::mutex mtx;

void f() {
    std::lock_guard<std::mutex> lock(mtx);   // lock() в конструкторе
    // критическая секция
}                                             // unlock() в деструкторе — ВСЕГДА, даже при исключении
```

C++17 (CTAD — выведение типа из конструктора, мы это разбирали):

```cpp
std::lock_guard lock(mtx);   // ✅ тип выводится
```

**Ограничения:** нельзя разблокировать раньше, нельзя перемещать, нельзя использовать с `condition_variable`. Самый дешёвый.

**Adopt-режим** — принять уже захваченный мьютекс:

```cpp
mtx.lock();
std::lock_guard lock(mtx, std::adopt_lock);   // не лочит, но разлочит в деструкторе
```

### `std::unique_lock` — гибкий

```cpp
std::unique_lock<std::mutex> lock(mtx);

lock.unlock();     // ✅ можно разблокировать досрочно
lock.lock();       // ✅ и снова заблокировать
lock.owns_lock();  // ✅ проверить состояние
auto l2 = std::move(lock);   // ✅ перемещаемый
```

**Режимы конструирования:**

```cpp
std::unique_lock lock(mtx);                     // сразу лочит
std::unique_lock lock(mtx, std::defer_lock);    // НЕ лочит — залочим позже
std::unique_lock lock(mtx, std::try_to_lock);   // try_lock()
std::unique_lock lock(mtx, std::adopt_lock);    // мьютекс уже залочен
```

**Когда нужен `unique_lock`:**

1. **С `condition_variable`** — обязателен (cv нужно уметь разлочивать/залочивать):

```cpp
std::unique_lock lock(mtx);
cv.wait(lock, []{ return ready; });   // ✅ lock_guard тут НЕ подойдёт
```

2. **Досрочная разблокировка** — сузить критическую секцию:

```cpp
void process() {
    std::unique_lock lock(mtx);
    auto data = getData();    // под локом
    lock.unlock();            // ✅ отпустили раньше
    expensiveWork(data);      // тяжёлая работа БЕЗ лока
}
```

3. **Возврат лока из функции** (перемещаемый):

```cpp
std::unique_lock<std::mutex> getLock() {
    std::unique_lock lock(mtx);
    return lock;   // ✅ move
}
```

4. **Отложенная блокировка** (для `std::lock`, см. ниже).

**Цена:** хранит флаг `owns_lock` → чуть больше и медленнее `lock_guard`. Используй `lock_guard`/`scoped_lock` по умолчанию, `unique_lock` — когда нужна гибкость.

### `std::scoped_lock` (C++17) — несколько мьютексов без deadlock

```cpp
std::mutex m1, m2;

std::scoped_lock lock(m1, m2);   // ✅ захватывает ОБА, deadlock-free (алгоритм избегания)
```

Внутри использует `std::lock` — алгоритм с откатами, гарантирующий отсутствие deadlock независимо от порядка захвата в разных потоках.

**`scoped_lock` — дефолтный выбор в C++17+**, он заменяет `lock_guard` (с одним мьютексом работает так же):

```cpp
std::scoped_lock lock(mtx);   // ✅ то же, что lock_guard, но универсальнее
```

### `std::shared_lock` (C++14) — разделяемая блокировка

Для `shared_mutex`, режим «читатель»:

```cpp
std::shared_lock lock(shared_mtx);   // много потоков одновременно
```

### Сводка

| |`lock_guard`|`unique_lock`|`scoped_lock`|`shared_lock`|
|---|---|---|---|---|
|Стандарт|C++11|C++11|**C++17**|C++14|
|Несколько мьютексов|❌|❌|✅ **deadlock-free**|❌|
|Досрочный unlock|❌|✅|❌|✅|
|Перемещаемый|❌|✅|❌|✅|
|С `condition_variable`|❌|✅|❌|✅ (`condition_variable_any`)|
|Оверхед|минимальный|небольшой|минимальный|—|

---

## Deadlock

**Четыре условия Коффмана** (все должны выполняться одновременно):

1. **Mutual exclusion** — ресурс захватывается эксклюзивно
2. **Hold and wait** — поток держит ресурс и ждёт другой
3. **No preemption** — ресурс нельзя отобрать
4. **Circular wait** — циклическая цепочка ожидания

Достаточно разорвать **любое** — deadlock невозможен.

### Классический deadlock

```cpp
std::mutex m1, m2;

void threadA() {
    std::lock_guard l1(m1);          // захватил m1
    std::this_thread::sleep_for(10ms);
    std::lock_guard l2(m2);          // ждёт m2
}

void threadB() {
    std::lock_guard l1(m2);          // захватил m2
    std::this_thread::sleep_for(10ms);
    std::lock_guard l2(m1);          // ждёт m1  → ⚠️ DEADLOCK (circular wait)
}
```

### Решение 1: `std::scoped_lock` / `std::lock` — снимает hold-and-wait

```cpp
void threadA() {
    std::scoped_lock lock(m1, m2);   // ✅ атомарно захватывает оба
}
void threadB() {
    std::scoped_lock lock(m2, m1);   // ✅ порядок НЕ важен — deadlock невозможен
}
```

Алгоритм внутри: пытается захватить по очереди, при неудаче **отпускает всё** и повторяет (с другого мьютекса). Hold-and-wait разорван.

До C++17 — `std::lock` + `defer_lock`:

```cpp
std::unique_lock l1(m1, std::defer_lock);
std::unique_lock l2(m2, std::defer_lock);
std::lock(l1, l2);   // ✅ атомарный захват
```

### Решение 2: строгий порядок захвата — снимает circular wait

Договориться, что мьютексы **всегда** берутся в одном порядке (например, по адресу):

```cpp
void transfer(Account& from, Account& to, int amount) {
    // ⚠️ без упорядочивания: transfer(a,b) и transfer(b,a) → deadlock
    std::scoped_lock lock(from.mtx_, to.mtx_);   // ✅ или scoped_lock решает это
    // ...
}
```

Ручное упорядочивание (если scoped_lock недоступен):

```cpp
std::mutex* first  = &from.mtx_ < &to.mtx_ ? &from.mtx_ : &to.mtx_;
std::mutex* second = &from.mtx_ < &to.mtx_ ? &to.mtx_   : &from.mtx_;
std::lock_guard l1(*first);
std::lock_guard l2(*second);   // ✅ порядок глобально консистентен
```

### Решение 3: `try_lock` с откатом

```cpp
while (true) {
    std::unique_lock l1(m1);
    std::unique_lock l2(m2, std::try_to_lock);
    if (l2.owns_lock()) break;    // ✅ оба захвачены
    l1.unlock();                   // откат — отпустили m1
    std::this_thread::yield();     // дали шанс другим
}
```

Риск **livelock** (потоки бесконечно откатываются, мешая друг другу) — нужен рандомизированный backoff.

### Решение 4: избегать вложенных локов вообще

Самое надёжное. **Не вызывай чужой код под локом** — он может залочить что-то ещё:

```cpp
void bad() {
    std::lock_guard lock(mtx_);
    callback_();   // ⚠️ ОПАСНО — callback может залочить другой мьютекс → deadlock
}

void good() {
    std::function<void()> cb;
    {
        std::lock_guard lock(mtx_);
        cb = callback_;   // копируем под локом
    }                     // отпустили
    cb();                 // ✅ вызываем БЕЗ лока
}
```

---

## Другие типичные ошибки

### 1. Забыть mutable для мьютекса

```cpp
class Counter {
    int value_ = 0;
    std::mutex mtx_;           // ⚠️ не mutable
public:
    int get() const {
        std::lock_guard lock(mtx_);   // ❌ ОШИБКА: lock() меняет mtx_, а метод const
        return value_;
    }
};
```

Решение — `mutable std::mutex mtx_;` (разбирали в теме const correctness).

### 2. Забыть имя переменной — временный lock_guard

```cpp
void f() {
    std::lock_guard<std::mutex>(mtx);   // ⚠️ ВРЕМЕННЫЙ! разрушается СРАЗУ → мьютекс отпущен
    criticalSection();                   // ⚠️ выполняется БЕЗ защиты!
}

void good() {
    std::lock_guard<std::mutex> lock(mtx);   // ✅ именованный объект живёт до конца scope
}
```

Классическая опечатка, компилятор молчит. (CTAD-форма `std::lock_guard(mtx);` — тоже создаёт временный.)

### 3. Слишком широкая критическая секция

```cpp
void bad() {
    std::lock_guard lock(mtx_);
    auto data = fetch();        // под локом
    expensiveCompute(data);     // ⚠️ 100 мс под локом — все потоки ждут!
    save(data);
}

void good() {
    Data data;
    {
        std::lock_guard lock(mtx_);
        data = fetch();          // ✅ только доступ к общему состоянию
    }
    expensiveCompute(data);      // ✅ без лока
    {
        std::lock_guard lock(mtx_);
        save(data);
    }
}
```

### 4. Возврат ссылки/указателя на защищённые данные

```cpp
class Registry {
    std::vector<int> data_;
    std::mutex mtx_;
public:
    std::vector<int>& get() {          // ⚠️ УТЕЧКА защищённых данных!
        std::lock_guard lock(mtx_);
        return data_;                   // ссылка живёт ПОСЛЕ разблокировки
    }
    std::vector<int> getCopy() {        // ✅ копия
        std::lock_guard lock(mtx_);
        return data_;
    }
};
```

То же — не передавай указатели на защищённые данные в колбэки/наружу.

### 5. Race condition ≠ data race

**Data race** — UB на уровне памяти (два потока пишут одну переменную без синхронизации). **Race condition** — логическая ошибка: результат зависит от порядка выполнения, **даже если каждая операция атомарна**.

```cpp
std::mutex mtx;
std::vector<int> v;

// ⚠️ RACE CONDITION, хотя data race нет — каждая операция под локом:
if (!v.empty()) {              // проверка (под локом внутри empty)
    // ⚠️ другой поток мог опустошить вектор ЗДЕСЬ
    int x = v.back();          // ⚠️ UB!
}

// ✅ проверка и действие — АТОМАРНО, под одним локом:
{
    std::lock_guard lock(mtx);
    if (!v.empty()) {
        int x = v.back();
        v.pop_back();
    }
}
```

Это TOCTOU (time-of-check to time-of-use). Именно поэтому у `std::stack` **нет** безопасного `pop()`, возвращающего значение — интерфейс `empty()`/`top()`/`pop()` принципиально не потокобезопасен как последовательность.

---

## Формулировки на собеседовании

**«Зачем `lock_guard`, если есть `lock()`/`unlock()`?»** — RAII: гарантированный `unlock()` при любом выходе, включая исключение. Ручной `unlock()` не выполнится при throw → мьютекс залочен навсегда.

**«Чем `unique_lock` отличается от `lock_guard`?»** — `unique_lock` поддерживает досрочный `unlock()`/повторный `lock()`, перемещаем, работает с `condition_variable`, поддерживает `defer_lock`/`try_to_lock`. Цена — хранит флаг владения (чуть дороже). `lock_guard` — минималистичный.

**«Что даёт `scoped_lock`?»** — Захват **нескольких** мьютексов **без deadlock** (алгоритм с откатами разрывает hold-and-wait). С одним мьютексом эквивалентен `lock_guard`. Дефолтный выбор в C++17+.

**«Условия возникновения deadlock?»** — Четыре условия Коффмана: mutual exclusion, hold-and-wait, no preemption, circular wait. Разрыв любого — deadlock невозможен.

**«Как избежать deadlock?»** — (1) `std::scoped_lock`/`std::lock` — атомарный захват нескольких. (2) Строгий глобальный порядок захвата. (3) `try_lock` с откатом (риск livelock). (4) Не звать чужой код под локом.

**«Разница data race и race condition?»** — Data race — **UB**: неатомарный конкурентный доступ к памяти без синхронизации. Race condition — **логическая** ошибка: результат зависит от планирования, даже когда каждая операция атомарна (TOCTOU). Мьютексы устраняют data race, но race condition требует правильного **гранулирования** критической секции.

**«Почему `std::mutex` нельзя лочить рекурсивно?»** — Не поддерживает — UB (обычно самоблокировка). Для рекурсии есть `recursive_mutex`, но его необходимость обычно указывает на проблему дизайна (лучше вынести приватный метод без лока).

**«Когда `shared_mutex`?»** — Много читателей, мало писателей. Читатели захватывают `shared_lock` параллельно, писатель — `unique_lock` эксклюзивно. При частых записях оверхед может не окупиться.

---

Отличие от Java: там `synchronized` — **встроенный в язык** реентерабельный (рекурсивный!) монитор на каждом объекте, с автоматическим release при выходе из блока/исключении — по сути RAII, встроенный в семантику. `ReentrantLock` — явный аналог `unique_lock` с `tryLock`, `lockInterruptibly`. `ReadWriteLock` ≈ `shared_mutex`. Ключевые различия: (1) в Java монитор **по умолчанию рекурсивен**, в C++ `std::mutex` — нет; (2) в Java data race не даёт UB — модель памяти гарантирует хотя бы «какое-то» значение (no out-of-thin-air), в C++ data race — **чистый UB**, компилятор вправе на нём оптимизировать что угодно; (3) в C++ нет `std::lock`-аналога в Java для атомарного захвата нескольких локов — там порядок соблюдают вручную.
