---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

`std::mutex` (mutual exclusion) используется для защиты общих данных от одновременного доступа из нескольких потоков.

## Пример без `std::mutex` - [[004 data race|data race]]

## Решение через `std::mutex`

```cpp
#include <iostream>
#include <thread>
#include <mutex>

struct Worker {
    static const size_t SIZE{100000};

    std::mutex mtx{};
    int value{};

    void run() {
        for (size_t i{}; i < SIZE; i++) {
            mtx.lock();
            ++value;
            mtx.unlock();
        }
    }
};

int main() {
    Worker w;
    std::thread t0 {&Worker::run, std::ref(w)};
    std::thread t1 {&Worker::run, std::ref(w)};

    t0.join();
    t1.join();

    std::cout << "w.value: " << w.value << std::endl;

    return 0;
}
```

```
w.value: 200000
```

## Почему `lock()` / `unlock()` не лучший вариант

Если между ними возникнет исключение:

```cpp
mtx.lock();
some_function(); // throw exception
mtx.unlock();    // will not executed
```

Мьютекс останется заблокированным навсегда.
