---
tags:
  - programming-language
  - cpp
  - RAII
---
[[RAII|<=]]

В C++ для безопасной и исключение-устойчивой работы с мьютексами используются **RAII-обёртки**, которые автоматически захватывают мьютекс при создании объекта и освобождают — при уничтожении (выходе из области видимости).

> ✅ Это гарантирует, что:
> - Мьютекс будет разблокирован даже при выбросе исключения,
> - Не будет deadlock'ов из-за забытого `unlock()`.


```cpp
#include <iostream>
#include <thread>
#include <mutex>

int main() {
    std::mutex mtx;
    int value;

    std::thread t1([&]() {
        // mutex is not captured yet
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);

        lock.lock();
        value = 1;
        std::cout << std::this_thread::get_id() << " set 1..." << std::endl;
        lock.unlock();

        lock.lock();
        value = 11;
        std::cout << std::this_thread::get_id() << " set 11..." << std::endl;
        lock.unlock();
    });

    std::thread t2([&]() {
        // mutex is not captured yet
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);

        lock.lock();
        value = 2;
        std::cout << std::this_thread::get_id() << " set 2..." << std::endl;
        lock.unlock();

        lock.lock();
        value = 22;
        std::cout << std::this_thread::get_id() << " set 22..." << std::endl;
        lock.unlock();
    });

    t1.join();
    t2.join();

    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "result => " << value << std::endl;

    return 0;
}
```

```
32432 set 1...
32432 set 11...
34072 set 2...
34072 set 22...
result => 22
```

### Также поддерживает:
- Отложенную блокировку (`std::defer_lock`)
- Попытку захвата (`try_lock`)
- Передачу владения между функциями
- Работу с `std::condition_variable`

```cpp
std::condition_variable cv;
std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock); // condition_variable сама управляет блокировкой
```
