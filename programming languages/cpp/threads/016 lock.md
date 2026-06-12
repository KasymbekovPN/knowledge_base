---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

### std::lock()

Захватывает **несколько мьютексов атомарно** без риска дедлока, используя алгоритм избегания взаимной блокировки. Доступен с C++11 — предшественник `scoped_lock` из C++17.

### Сигнатура

```cpp
template<class Lockable1, class Lockable2, class... LockableN>
void std::lock(Lockable1&, Lockable2&, LockableN&...);
```

```cpp
#include <iostream>
#include <mutex>

struct Account {
    std::mutex mtx;
    int balance;

    Account(const int _balance): balance{_balance} {}
};

std::ostream& operator<<(std::ostream& _os, const Account& _account) {
    return _os << "{" << _account.balance << "}";
}

void safe_transfer(Account& _a, Account& _b, const int amount) {
    std::lock(_a.mtx, _b.mtx);

    std::lock_guard lock1(_a.mtx, std::adopt_lock); // take ownship
    std::lock_guard lock2(_b.mtx, std::adopt_lock); // without lock

    _a.balance -= amount;
    _b.balance += amount;
}

int main() {
    Account a{100};
    Account b{200};

    safe_transfer(a, b, 50);

    std::cout << "A: " << a << "\n";
    std::cout << "B: " << b << "\n";

    return 0;
}
```

```
A: {50}
B: {250}
```

### Сравнение с scoped_lock

```cpp
// C++11 — std::lock + adopt_lock
std::lock(mtx1, mtx2);
std::lock_guard l1(mtx1, std::adopt_lock);
std::lock_guard l2(mtx2, std::adopt_lock);

// C++17 — то же самое, одной строкой
std::scoped_lock lock(mtx1, mtx2);
```

`scoped_lock` внутри делает то же самое — просто удобнее. `std::lock` нужен когда требуется совместимость с C++11/14 или ручной контроль над `unique_lock`.

### С unique_lock — для condition_variable

`unique_lock` нужен если после захвата потребуется `wait`:

```cpp
std::unique_lock l1(mtx1, std::defer_lock); // не захватывать сразу
std::unique_lock l2(mtx2, std::defer_lock);

std::lock(l1, l2); // захватить оба безопасно
```

`std::defer_lock` — создать `unique_lock` без захвата, чтобы передать его в `std::lock`.
