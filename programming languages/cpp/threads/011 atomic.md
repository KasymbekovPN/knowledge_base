---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

### std::atomic

Обёртка над переменной, операции над которой выполняются **атомарно** — неделимо с точки зрения других потоков. Не требует мьютекса для простых операций.

### Основные методы

```cpp
std::atomic<int> a{0};

a.store(5);          // записать
int x = a.load();    // прочитать
a.fetch_add(1);      // прибавить, вернуть старое значение
a.fetch_sub(1);      // вычесть, вернуть старое значение
++a; --a;            // атомарные инкремент/декремент

// compare and swap — если a == expected, записать desired
int expected = 5;
a.compare_exchange_strong(expected, 10);
```

### atomic vs mutex

```cpp
// mutex — универсально, но тяжелее
std::mutex mtx;
std::lock_guard lock(mtx);
++counter;

// atomic — легче, но только для простых типов и операций
++counter; // атомарно
```

|                   | `atomic`                       | `mutex`                      |
| ----------------- | ------------------------------ | ---------------------------- |
| Накладные расходы | минимальные                    | выше                         |
| Применимость      | простые типы, простые операции | любые данные, сложная логика |
| Сложные операции  | нет                            | да                           |

### Для каких типов работает

```cpp
std::atomic<int>    // да
std::atomic<bool>   // да
std::atomic<float>  // да (C++20 — fetch_add и др.)
std::atomic<T*>     // да — атомарные указатели

std::atomic<std::string> // нет — тип должен быть trivially copyable
```

### Правило выбора

- Счётчики, флаги, указатели → **`atomic`**
- Несколько переменных меняются вместе, сложная логика → **`mutex

```cpp
#include <iostream>
#include <thread>
#include <atomic>

struct NonAtomicValue {
    inline static constexpr size_t SIZE{1000000};
    int value{};

    void increment() {
        for (size_t i{}; i < SIZE; ++i) {
            ++value;
        }
    }
};

struct AtomicValue {
    inline static constexpr size_t SIZE{1000000};
    std::atomic<int> value{0};

    void increment() {
        for (size_t i{}; i < SIZE; ++i) {
            ++value;
        }
    }
};

template<typename T>
concept HasValue = requires(const T& t) {
    t.value;
};

template<HasValue T>
std::ostream& operator<<(std::ostream& _os, const T& _input) {
    return _os << "{" << _input.value << "}";
}

int main() {
    NonAtomicValue nav;
    std::thread t0 {&NonAtomicValue::increment, std::ref(nav)};
    std::thread t1 {&NonAtomicValue::increment, std::ref(nav)};
    t0.join();
    t1.join();

    AtomicValue av;
    std::thread t2 {&AtomicValue::increment, std::ref(av)};
    std::thread t3 {&AtomicValue::increment, std::ref(av)};
    t2.join();
    t3.join();

    std::cout << "nav: " << nav << std::endl;
    std::cout << "av: " << av << std::endl;

    return 0;
}
```

```
nav: {128143}
av: {200000}
```
