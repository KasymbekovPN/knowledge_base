[[raw data/cpp/interview/_|<=]]

# `std::thread`, `jthread`, `async`/`future`/`promise`

## `std::thread` — низкоуровневый поток

```cpp
#include <thread>

void work(int x) { std::cout << x; }

std::thread t(work, 42);   // поток СТАРТУЕТ немедленно при конструировании
t.join();                  // ждём завершения
```

### Обязательное правило: `join()` или `detach()`

Если объект `std::thread` разрушается, будучи **joinable** (поток запущен и не присоединён/не отсоединён) → вызывается **`std::terminate()`** — программа падает.

```cpp
void bad() {
    std::thread t(work, 42);
    // ⚠️ выход из области → ~thread() при joinable → std::terminate()!
}

void good() {
    std::thread t(work, 42);
    t.join();   // ✅
}
```

Почему так сурово: стандарт не мог выбрать разумный дефолт. Тихо `join()` в деструкторе → неожиданные зависания. Тихо `detach()` → поток продолжает работать с уже разрушенными данными. Комитет решил: пусть падает явно.

**Ловушка с исключениями:**

```cpp
void risky() {
    std::thread t(work, 42);
    mayThrow();     // ⚠️ если бросит → t.join() не выполнится → terminate!
    t.join();
}
```

Решение — RAII-обёртка (thread_guard):

```cpp
class ThreadGuard {
    std::thread t_;
public:
    explicit ThreadGuard(std::thread t) : t_(std::move(t)) { }
    ~ThreadGuard() { if (t_.joinable()) t_.join(); }   // ✅ join при любом выходе
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};
```

Ровно эту обёртку C++20 и стандартизировал — это `std::jthread`.

### `detach()` — отсоединение

```cpp
std::thread t(work, 42);
t.detach();   // поток продолжает жить сам по себе; t больше не joinable
```

**Опасно:** отсоединённый поток может пережить объекты, на которые ссылается:

```cpp
void danger() {
    int local = 42;
    std::thread t([&local]{ 
        std::this_thread::sleep_for(1s);
        std::cout << local;   // ⚠️ UB! local давно умер
    });
    t.detach();
}   // функция вышла, local разрушен, а поток ещё работает
```

Практика: `detach()` используется редко (фоновые демоны). По умолчанию — `join()`.

### Передача аргументов — копируются по значению!

```cpp
void modify(int& x) { x = 42; }

int value = 0;
std::thread t(modify, value);            // ❌ ОШИБКА КОМПИЛЯЦИИ — value копируется,
                                          //    копию нельзя привязать к int&
std::thread t(modify, std::ref(value));  // ✅ std::ref — передать по ссылке
t.join();
```

`std::thread` **всегда копирует** (или перемещает) аргументы в своё внутреннее хранилище — чтобы они пережили выход из вызывающей функции. Для передачи по ссылке нужен явный `std::ref`/`std::cref`.

Для move-only типов:

```cpp
auto p = std::make_unique<Widget>();
std::thread t(consume, std::move(p));   // ✅ перемещение
```

### Move-семантика потока

`std::thread` **некопируем**, но перемещаем (владение одно):

```cpp
std::thread t1(work, 1);
std::thread t2 = std::move(t1);   // ✅ t1 больше не joinable
// std::thread t3 = t1;           // ❌ копирование запрещено

std::vector<std::thread> threads;
for (int i = 0; i < 4; ++i)
    threads.emplace_back(work, i);   // ✅ вектор потоков
for (auto& t : threads) t.join();
```

### Полезное

```cpp
std::thread::hardware_concurrency();   // подсказка: сколько потоков имеет смысл (может вернуть 0!)
std::this_thread::get_id();
std::this_thread::sleep_for(100ms);
std::this_thread::yield();             // отдать квант времени
```

---

## `std::jthread` (C++20) — «joining thread»

Два улучшения над `std::thread`:

### 1. Автоматический `join()` в деструкторе

```cpp
void good() {
    std::jthread t(work, 42);
    mayThrow();   // ✅ даже при исключении — деструктор сделает join
}                 // ✅ автоматический join, никакого terminate
```

Это RAII, применённый к потоку. **`jthread` — дефолтный выбор в C++20+.**

### 2. Кооперативная отмена через `stop_token`

```cpp
void worker(std::stop_token st, int id) {
    while (!st.stop_requested()) {   // ← проверяем запрос на остановку
        doWork();
        std::this_thread::sleep_for(10ms);
    }
    std::cout << "worker " << id << " stopped\n";
}

std::jthread t(worker, 1);   // stop_token передаётся АВТОМАТИЧЕСКИ первым аргументом
// ...
t.request_stop();            // просим остановиться
// деструктор: request_stop() + join()
```

Если первый параметр функции — `std::stop_token`, `jthread` сам его подставляет.

**Важно: отмена кооперативная**, а не принудительная. Поток должен **сам** проверять `stop_requested()`. Принудительно убить поток в C++ нельзя (и ни в одном вменяемом языке — состояние осталось бы разрушенным: незакрытые мьютексы, недоаллоцированная память).

### `stop_callback` — реакция на отмену

```cpp
std::jthread t([](std::stop_token st) {
    std::stop_callback cb(st, []{ 
        std::cout << "cancellation requested!\n";   // вызовется при request_stop()
    });
    // ... работа
});
```

Полезно для пробуждения из блокирующих ожиданий (см. `condition_variable_any` ниже).

---

## `std::async` / `std::future` / `std::promise` — асинхронные результаты

Более высокоуровневая абстракция: не «поток», а **асинхронное вычисление, возвращающее значение**.

### `std::async` — запустить и получить future

```cpp
#include <future>

int compute(int x) { return x * x; }

std::future<int> f = std::async(compute, 5);
// ... делаем что-то ещё параллельно ...
int result = f.get();   // блокируется до готовности, возвращает 25
```

`future<T>` — «обещание значения типа T в будущем».

### Политики запуска — критично!

```cpp
std::async(std::launch::async, compute, 5);      // ✅ ГАРАНТИРОВАННО новый поток
std::async(std::launch::deferred, compute, 5);   // ЛЕНИВО: выполнится при get(), в ТЕКУЩЕМ потоке
std::async(compute, 5);                           // ⚠️ ДЕФОЛТ: async|deferred — на усмотрение реализации!
```

**Дефолтная политика — ловушка.** Реализация вправе выбрать `deferred` (например, при нехватке потоков), и тогда «асинхронное» вычисление не запустится, пока не позовёшь `get()`:

```cpp
auto f = std::async(longTask);   // может НЕ стартовать!
doOtherWork();                    // ожидали параллелизм — а его нет
f.get();                          // только ЗДЕСЬ начнётся longTask (если deferred)
```

**Правило: всегда указывай `std::launch::async` явно**, если нужен настоящий параллелизм.

### Ловушка: деструктор future от `async` блокируется

```cpp
{
    std::async(std::launch::async, longTask);   // ⚠️ временный future!
}   // деструктор future БЛОКИРУЕТСЯ до завершения longTask → фактически синхронно!

// то же самое:
std::async(std::launch::async, task1);   // блокируется до завершения task1
std::async(std::launch::async, task2);   // ...потом task2 — НИКАКОГО параллелизма
```

Причина: `future`, полученный от `std::async` (и только от него), в деструкторе **ждёт** завершения задачи. Это единственное исключение — future от `promise`/`packaged_task` так не делает.

**Правильно — сохранить future:**

```cpp
auto f1 = std::async(std::launch::async, task1);
auto f2 = std::async(std::launch::async, task2);   // ✅ оба работают параллельно
f1.get(); f2.get();
```

### `std::future` — API

```cpp
std::future<int> f = std::async(std::launch::async, compute, 5);

f.get();               // блокируется, возвращает результат. Можно вызвать ТОЛЬКО ОДИН РАЗ!
f.wait();              // блокируется, не забирая результат
f.wait_for(100ms);     // → future_status::ready / timeout / deferred
f.wait_until(tp);
f.valid();             // false после get()
```

**`get()` — одноразовый:** после него future становится невалидным (результат перемещён). Повторный `get()` → UB/исключение.

Нужно несколько потребителей → `std::shared_future`:

```cpp
std::shared_future<int> sf = f.share();
int a = sf.get();   // ✅ можно много раз
int b = sf.get();   // ✅
```

### Исключения через future

Если задача бросила исключение, оно **сохраняется** и перебрасывается при `get()`:

```cpp
auto f = std::async(std::launch::async, []{ throw std::runtime_error("oops"); });
try {
    f.get();   // ✅ исключение перебрасывается ЗДЕСЬ, в вызывающем потоке
} catch (const std::exception& e) {
    std::cout << e.what();   // "oops"
}
```

Это огромное преимущество над `std::thread`: исключение, вылетевшее из функции потока, вызывает **`std::terminate()`** — его никак не поймать. `future` решает эту проблему.

### `std::promise` — ручная установка результата

`promise` — «пишущий конец», `future` — «читающий». Разделены, чтобы можно было передать результат из произвольного места.

```cpp
std::promise<int> p;
std::future<int> f = p.get_future();   // связанный future

std::thread t([&p] {
    try {
        int result = compute();
        p.set_value(result);            // ✅ передаём значение
    } catch (...) {
        p.set_exception(std::current_exception());   // ✅ передаём исключение
    }
});

int r = f.get();   // получаем (или ловим исключение)
t.join();
```

Когда нужен `promise`, а не `async`: результат готовится не «одной функцией», а по ходу работы потока; или нужно передать сигнал/значение между потоками, не привязываясь к вызову функции.

**Правило: `set_value` можно вызвать только один раз.** Повторный → `std::future_error`.

Если promise разрушен без установки значения → future получает исключение `std::broken_promise`.

### `std::packaged_task` — обёртка вызываемого объекта

```cpp
std::packaged_task<int(int)> task(compute);
std::future<int> f = task.get_future();

std::thread t(std::move(task), 5);   // ✅ task — move-only, запускаем в потоке
t.join();

int r = f.get();   // 25
```

Полезно для **thread pool**: задачи кладутся в очередь, воркеры их разбирают, а клиент получает future:

```cpp
class ThreadPool {
    std::queue<std::packaged_task<void()>> tasks_;
    // ...
public:
    template<class F>
    auto submit(F f) -> std::future<std::invoke_result_t<F>> {
        std::packaged_task<std::invoke_result_t<F>()> task(std::move(f));
        auto fut = task.get_future();
        {
            std::lock_guard lock(mtx_);
            tasks_.push(std::move(task));
        }
        cv_.notify_one();
        return fut;
    }
};
```

---

## Сравнение подходов

| |`std::thread` / `jthread`|`std::async`|`promise`/`future`|`packaged_task`|
|---|---|---|---|---|
|Уровень|низкий|высокий|средний|средний|
|Возврат значения|❌ (вручную)|✅ future|✅ future|✅ future|
|Исключения|**terminate!**|✅ через `get()`|✅ `set_exception`|✅|
|Контроль над потоком|полный|нет|—|—|
|Когда использовать|нужен контроль (affinity, приоритет, долгоживущий воркер)|простое «посчитай параллельно»|результат из произвольного места|thread pool|

**Практическая рекомендация:**

- Нужно вычислить что-то параллельно и получить результат → **`std::async(std::launch::async, ...)`**
- Нужен долгоживущий воркер / полный контроль → **`std::jthread`** (C++20) или `std::thread` + RAII-guard
- Строишь thread pool → **`packaged_task`** + очередь
- Нужно просигналить значение между потоками вне модели «функция→результат» → **`promise`**

---

## Формулировки на собеседовании

**«Что будет, если не вызвать `join()` или `detach()`?»** — Деструктор `std::thread` при joinable вызывает **`std::terminate()`** — программа падает. Комитет намеренно не выбрал дефолт: тихий join → зависания, тихий detach → доступ к разрушенным данным.

**«Чем `jthread` лучше `thread`?»** — (1) Автоматический `join()` в деструкторе (RAII, безопасно при исключениях). (2) Кооперативная отмена через `stop_token`/`request_stop()`.

**«Можно ли принудительно убить поток?»** — Нет, и это правильно: состояние осталось бы разрушенным (залоченные мьютексы, недоинициализированные объекты). Только **кооперативная** отмена — поток сам проверяет флаг/`stop_token`.

**«Что не так с `std::async` без указания политики?»** — Дефолт `async|deferred` — реализация вправе выбрать `deferred`, и задача не запустится до `get()`. Всегда указывай `std::launch::async` явно.

**«Почему `std::async(...)` без сохранения future работает синхронно?»** — Деструктор future, полученного **от `async`**, блокируется до завершения задачи. Временный future разрушается в конце выражения → блокировка. Нужно сохранять future в переменную.

**«Как обработать исключение из потока?»** — В `std::thread` — никак: исключение из функции потока → `std::terminate()`. Через `future` (`async`/`promise`/`packaged_task`) исключение сохраняется и **перебрасывается при `get()`** в вызывающем потоке.

**«Почему `std::thread(f, x)` копирует x?»** — Аргументы копируются во внутреннее хранилище потока, чтобы пережить выход вызывающей функции. Для передачи по ссылке нужен `std::ref`.

**«Сколько раз можно вызвать `future::get()`?»** — Один. После — future невалиден. Для нескольких потребителей — `shared_future`.

---

Отличие от Java: там `Thread` не требует join (GC разберётся, а поток просто отработает); есть `ExecutorService` + `Future` — прямой аналог `async`/`future`, и `CompletableFuture` — мощнее C++ `future` (композиция: `thenApply`, `thenCombine` — в C++ этого **нет** до C++23/`std::execution`). Зато в Java `Thread.stop()` **deprecated** ровно по той же причине, что и в C++ нет принудительного убийства — небезопасно. Ещё различие: в Java исключение из потока не роняет программу (идёт в `UncaughtExceptionHandler`), в C++ — `std::terminate()`. Это принципиально: в C++ **обязательно** оборачивать тело потока в try/catch или использовать future.
