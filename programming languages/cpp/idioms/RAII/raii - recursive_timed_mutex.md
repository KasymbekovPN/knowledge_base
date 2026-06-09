---
tags:
  - programming-language
  - cpp
  - RAII
---
[[RAII|<=]]

`std::recursive_timed_mutex` — это **гибридный мьютекс** из заголовочного файла `<mutex>`, который объединяет возможности двух других типов:

- `std::recursive_mutex` — можно захватывать **многократно одним и тем же потоком**,
- `std::timed_mutex` — поддерживает **таймауты** при попытке захвата (`try_lock_for`, `try_lock_until`).

> 🔧 Это самый "тяжёлый", но и самый гибкий тип стандартного мьютекса в C++.

### Основные методы

| Метод | Назначение |
|------|-----------|
| `.lock()` | Блокирует мьютекс (может зависнуть) |
| `.try_lock()` | Пытается захватить немедленно → `true`/`false` |
| `.try_lock_for(rel_time)` | Ждёт указанное время (например, `100ms`) |
| `.try_lock_until(abs_time)` | Ждёт до определённого момента времени |
| `.unlock()` | Разблокирует мьютекс (вызывается столько раз, сколько было `lock()`) |

> ⚠️ Каждый `lock()` должен быть сопровождён `unlock()` — внутренний счётчик отслеживает глубину рекурсии.

### Важные моменты

| Особенность                                   | Объяснение                                    |
| --------------------------------------------- | --------------------------------------------- |
| Доступен с C++11                              | Часть стандартной библиотеки                  |
| Может быть захвачен многократно одним потоком | Счётчик увеличивается при каждом `lock()`     |
| Поддерживает таймауты                         | `try_lock_for`, `try_lock_until`              |
| Медленнее всех мьютексов                      | Из-за сложной внутренней логики               |
| Требует больше памяти                         | Хранит ID потока, счётчик, состояние ожидания |
| Не используйте без необходимости              | Часто можно обойтись простым `std::mutex`     |

### Сравнение всех стандартных мьютексов

| Мьютекс                           | Рекурсивный? | Таймауты? | Когда использовать                |
| --------------------------------- | ------------ | --------- | --------------------------------- |
| `std::mutex`                      | ❌            | ❌         | По умолчанию                      |
| `std::recursive_mutex`            | ✅            | ❌         | При рекурсивных вызовах           |
| `std::timed_mutex`                | ❌            | ✅         | При необходимости таймаутов       |
| `std::recursive_timed_mutex`      | ✅            | ✅         | При **рекурсии + таймаутах**      |
| `std::shared_mutex` (C++17)       | ❌            | ❌         | Для read/write (shared/exclusive) |
| `std::shared_timed_mutex` (C++14) | ❌            | ✅         | Для shared/exclusive + таймауты   |

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

void test(int, int);

std::recursive_timed_mutex mtx;

int main() {
    std::thread t1(test, 1, 1);
    std::thread t2(test, 1, 2);

    t1.join();
    t2.join();

    return 0;
}

void test(int _depth, int _id) {
    if (!mtx.try_lock_for(std::chrono::milliseconds(200))) {
        std::cout
	        << "[" << _id << "] Timeout at depth "
	        << _depth  << std::endl;
        return;
    }

    std::cout << "[" << _id << "] Locked at depth " << _depth << std::endl;
    if (_depth < 3) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        test(_depth + 1, _id);
    }

    mtx.unlock();
}
```

```
[1] Locked at depth 1
[1] Locked at depth 2
[2] Timeout at depth 1
[1] Locked at depth 3
```
