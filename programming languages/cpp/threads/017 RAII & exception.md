---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

### Проблема — исключение без RAII

```cpp
std::mutex mtx;

void worker() {
    mtx.lock();
    do_work();       // бросает исключение
    mtx.unlock();    // никогда не выполнится — мьютекс навсегда заблокирован
}
```

В однопоточном коде утечка ресурса неприятна. В многопоточном — **дедлок**: другой поток никогда не получит мьютекс.

### RAII решает это автоматически

Деструктор вызывается при выходе из области видимости **в любом случае** — и при нормальном возврате, и при исключении:

```cpp
void worker() {
    std::lock_guard lock(mtx); // захват
    do_work();                 // бросает исключение
}                              // деструктор lock — unlock() в любом случае
```

### Исключение из потока — что происходит

Если исключение **не поймано** внутри потока — программа завершается через `std::terminate`:

```cpp
std::thread t([]() {
    throw std::runtime_error("oops"); // std::terminate — крах всей программы
});
t.join();
```

Поэтому в потоке нужно ловить на верхнем уровне:

```cpp
std::thread t([]() {
    try {
        do_work();
    } catch (const std::exception& e) {
        std::cerr << "thread error: " << e.what() << '\n';
    }
});
t.join();
```

### join() и RAII

`std::thread` не делает `join()` автоматически — если объект уничтожается без `join()`/`detach()`, вызывается `std::terminate`:

```cpp
void bad() {
    std::thread t(worker);
    throw std::runtime_error("oops"); // t уничтожается без join → terminate
}
```

RAII-обёртка — `std::jthread` (C++20) вызывает `join()` в деструкторе автоматически:

```cpp
void good() {
    std::jthread t(worker);
    throw std::runtime_error("oops"); // деструктор jthread вызовет join()
}
```

```cpp
#include <iostream>
#include <thread>
#include <exception>
#include <future>

std::exception_ptr thread_exception;

void worker() {
    try {
        throw std::runtime_error("fail in thread");
    } catch(...) {
        thread_exception = std::current_exception();
    }
}

int main() {
    std::thread t0{worker};
    t0.join();

    if (thread_exception) {
        try {
            std::rethrow_exception(thread_exception);
        } catch(const std::exception& e) {
            std::cerr << "[0] caught: " << e.what() << '\n';
        }
    }

    auto&& f = std::async(std::launch::async, []() -> int {
        throw std::runtime_error("fail");
    });
    try {
        f.get();
    } catch(const std::exception& e) {
        std::cerr << "[1] caught: " << e.what() << '\n';
    }

    return 0;
}
```

```
[0] caught: fail in thread
[1] caught: fail
```
