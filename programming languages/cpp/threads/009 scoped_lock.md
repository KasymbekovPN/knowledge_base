---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

Появился в C++17. RAII-обёртка, которая захватывает **несколько мьютексов одновременно** без риска дедлока. При выходе из области видимости освобождает все.

### Проблема без scoped_lock — дедлок

```cpp
std::mutex mtx1, mtx2;

// поток A                   // поток B
mtx1.lock();                 mtx2.lock();
mtx2.lock(); // ждёт B       mtx1.lock(); // ждёт A  ← дедлок
```

Оба потока ждут друг друга бесконечно.

### Решение — std::scoped_lock

```cpp
std::scoped_lock lock(mtx1, mtx2); // захватывает оба атомарно
```

Внутри использует алгоритм, исключающий дедлок (аналог `std::lock`), независимо от порядка передачи мьютексов.

**Правило:** если нужно захватить 2+ мьютекса — всегда `scoped_lock`.

### Сравнение с альтернативами

| `lock_guard`      | `unique_lock` | `scoped_lock`     |               |
| ----------------- | ------------- | ----------------- | ------------- |
| Мьютексов         | 1             | 1                 | **несколько** |
| Ручной unlock     | нет           | да                | нет           |
| Защита от дедлока | нет           | через `std::lock` | **встроена**  |
| C++               | 11            | 11                | **17**        |

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

struct Account {
    int balance{};
    std::mutex mtx;

    Account(const int _balance): balance{_balance} {}
};

void transfer_ab(Account& _a, Account& _b, int amount) {
    std::scoped_lock lock{_a.mtx, _b.mtx};
    _a.balance -= amount;
    _b.balance += amount;
}

void transfer_ba(Account& _a, Account& _b, int amount) {
    std::scoped_lock lock{_b.mtx, _a.mtx};
    _b.balance -= amount;
    _a.balance += amount;
}

int main() {
    Account alice{1000};
    Account bob{900};

    std::vector<std::thread> threads;
    for (size_t i{}; i < 50; ++i) {
        threads.emplace_back(std::thread(transfer_ab, std::ref(alice), std::ref(bob), 1));
        threads.emplace_back(std::thread(transfer_ba, std::ref(alice), std::ref(bob), 2));
    }

    for (auto &&t: threads) {
        t.join();
    }

    std::cout << "Alice " << alice.balance << std::endl;
    std::cout << "Bob " << bob.balance << std::endl;

    return 0;
}
```

```
Alice 1050
Bob 850
```
