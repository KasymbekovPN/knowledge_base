---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

Это RAII-обёртка над мьютексом. Захватывает мьютекс в конструкторе и **автоматически освобождает в деструкторе** — при выходе из области видимости (в том числе при исключении).

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
            std::lock_guard<std::mutex> lock(mtx);
            ++value;
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

Без мьютекса `++counter` — **гонка данных** (UB), результат непредсказуем.

### Ключевые свойства

| Свойство          | Значение                               |
| ----------------- | -------------------------------------- |
| Захват            | в конструкторе                         |
| Освобождение      | в деструкторе                          |
| Ручное unlock     | невозможно                             |
| Передача владения | невозможна (non-copyable, non-movable) |
