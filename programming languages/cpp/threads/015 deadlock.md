---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

### Deadlock

Ситуация когда два или более потока **бесконечно ждут друг друга**, удерживая ресурсы, нужные другим.

### Классический сценарий

```cpp
std::mutex mtx1, mtx2;

// Поток A          // Поток B
mtx1.lock();        mtx2.lock();
// ...              // ...
mtx2.lock();        mtx1.lock();  // ← дедлок
```

Поток A держит `mtx1` и ждёт `mtx2`. Поток B держит `mtx2` и ждёт `mtx1`. Оба ждут вечно.

### Четыре необходимых условия (условия Коффмана)

Дедлок возможен только если **все четыре** выполняются одновременно:

1. **Взаимное исключение** — ресурс занят одним потоком
2. **Удержание и ожидание** — поток держит один ресурс и ждёт другой
3. **Отсутствие вытеснения** — ресурс нельзя забрать принудительно
4. **Циклическое ожидание** — A ждёт B, B ждёт A

Достаточно устранить **одно** условие — дедлока не будет.

### Алгоритм упорядочения мьютексов

Самый простой способ устранить **циклическое ожидание**: все потоки захватывают мьютексы **в одном и том же порядке**.

```cpp
std::mutex mtx1, mtx2;

void thread_a() {
    std::lock_guard lock1(mtx1); // сначала mtx1
    std::lock_guard lock2(mtx2); // потом mtx2
}

void thread_b() {
    std::lock_guard lock1(mtx1); // тот же порядок — дедлока нет
    std::lock_guard lock2(mtx2);
}
```

Проблема — порядок нужно соблюдать **вручную** во всём коде. Легко нарушить.

### Когда порядок неизвестен заранее — std::scoped_lock

Типичный пример: функция `transfer(from, to)` — порядок аргументов меняется.

```cpp
struct Account {
    int balance{};
    std::mutex mtx;
};

// ОПАСНО — порядок захвата зависит от аргументов
void transfer_unsafe(Account& from, Account& to, int amount) {
    std::lock_guard lock1(from.mtx); // поток A: from=alice
    std::lock_guard lock2(to.mtx);   // поток B: from=bob → дедлок
    from.balance -= amount;
    to.balance   += amount;
}

// БЕЗОПАСНО — scoped_lock захватывает атомарно
void transfer_safe(Account& from, Account& to, int amount) {
    std::scoped_lock lock(from.mtx, to.mtx); // порядок не важен
    from.balance -= amount;
    to.balance   += amount;
}
```

### Упорядочение по адресу — когда scoped_lock недоступен

Если нужен явный контроль — упорядочить по адресу объекта:

```cpp
void transfer(Account& a, Account& b, int amount) {
    auto* first  = &a < &b ? &a : &b;
    auto* second = &a < &b ? &b : &a;

    std::lock_guard lock1(first->mtx);
    std::lock_guard lock2(second->mtx);

    a.balance -= amount;
    b.balance += amount;
}
```

Оба потока всегда захватят мьютекс объекта с **меньшим адресом** первым — цикл невозможен.

### Итог

|Подход|Когда применять|
|---|---|
|Фиксированный порядок вручную|простые случаи, порядок очевиден|
|`std::scoped_lock`|захват 2+ мьютексов, C++17|
|Упорядочение по адресу|нужен ручной контроль без scoped_lock|
