---
tags:
  - programming-language
  - cpp
  - RAII
---
[[RAII|<=]]

`std::timed_mutex` — это тип мьютекса из заголовочного файла `<mutex>`, который **позволяет захватывать блокировку с таймаутом**.  
Он расширяет поведение `std::mutex`, добавляя возможность ждать блокировку не бесконечно, а ограниченное время.

> ✅ Это полезно, когда вы хотите:
> - Избежать deadlock'ов,
> - Реализовать тайм-ауты,
> - Проверить доступность ресурса без ожидания.

## 🔍 Основные методы

| Метод | Назначение |
|------|-----------|
| `.lock()` | Блокирует мьютекс (может зависнуть, если занят) |
| `.try_lock()` | Пытается захватить немедленно → `true`/`false` |
| `.try_lock_for(rel_time)` | Ждёт указанное время (`std::chrono::seconds`, `ms` и т.д.) |
| `.try_lock_until(abs_time)` | Ждёт до определённого момента времени |
| `.unlock()` | Разблокирует мьютекс |

## ⚠️ Важные моменты

| Особенность | Объяснение |
|-------------|------------|
| Не может быть использован с `std::lock_guard` | Только с `std::unique_lock` для таймаутов |
| Требует `<chrono>` для работы со временем | `milliseconds`, `seconds` и т.д. |
| Может выбросить исключение при превышении лимита потоков | Редко, но возможно |
| Медленнее, чем `std::mutex` | Из-за дополнительной логики |
| Подходит для защиты критических секций с ограничением по времени | Например, UI, реальное время |

## 🆚 Сравнение мьютексов

| Мьютекс | Описание | Поддерживает таймаут? |
|--------|---------|------------------------|
| `std::mutex` | Базовый мьютекс | ❌ Нет |
| `std::recursive_mutex` | Можно захватывать многократно одним потоком | ❌ Нет |
| `std::timed_mutex` | Поддерживает `try_lock_for`, `try_lock_until` | ✅ Да |
| `std::shared_timed_mutex` (C++14) | Читатели/писатели + таймауты | ✅ Да |

### try_lock_for

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::timed_mutex tmx;

void worker(int);

int main() {
    // tmx.lock();

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    tmx.unlock();

    t1.join();
    t2.join();

    return 0;
}

void worker(int id) {
    if (tmx.try_lock_for(std::chrono::milliseconds(100))) {
        std::cout << "Worker " << id << " got the lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        tmx.unlock();
    } else {
        std::cout << "Worker " << id << " timed out" << std::endl;
    }
}
```

```
Worker 1 got the lock
Worker 2 timed out
```


### try_lock_until

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::timed_mutex tmx;

void worker(int, int);

int main() {
    std::thread t1(worker, 1, 200);
    std::thread t2(worker, 2, 300);

    t1.join();
    t2.join();

    return 0;
}

void worker(int id, int ms) {
    auto deadline =
        std::chrono::steady_clock::now() +
        std::chrono::milliseconds(ms);

    if (tmx.try_lock_until(deadline)) {
        std::cout << "Worker " << id << " got the lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        tmx.unlock();
    } else {
        std::cout << "Worker " << id << " timed out" << std::endl;
    }
}
```

```
Worker 1 got the lock
Worker 2 timed out
```
