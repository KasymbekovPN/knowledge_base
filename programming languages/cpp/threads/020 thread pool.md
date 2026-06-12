---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

## Пул потоков (Thread Pool)

Паттерн при котором создаётся **фиксированное число потоков** заранее, и они многократно переиспользуются для выполнения задач из очереди. Избегает дорогостоящего создания/уничтожения потока для каждой задачи.

### Почему не создавать поток под каждую задачу

```cpp
// плохо — создание потока дорого (память стека, syscall)
for (auto& task : tasks)
    std::thread(task).detach();
```

Создание потока занимает ~1–10 мкс и ~1MB памяти под стек. При сотнях задач это накладные расходы.

### Ключевые моменты реализации

| Элемент                      | Роль                                               |
| ---------------------------- | -------------------------------------------------- |
| `condition_variable`         | потоки спят пока нет задач                         |
| `stop` флаг                  | сигнал завершения при деструкции                   |
| `notify_all()` в деструкторе | разбудить все спящие потоки                        |
| `packaged_task`              | связать задачу с `future` для получения результата |

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <format>

class ThreadPool {

public:
    explicit ThreadPool(const size_t num_threads) {
        for (size_t i{}; i < num_threads; ++i) {
            workers.emplace_back([this](){
                loop();
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock lock{mtx};
            stop = true;
        }
        cv.notify_all();

        for (auto &worker: workers) {
            worker.join();
        }
    }

    void submit(std::function<void()> task) {
        {
            std::unique_lock lock{mtx};
            queue.push(std::move(task));
        }
        cv.notify_one();
    }

private:
    void loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock lock{mtx};
                cv.wait(lock, [this]() {
                    return stop || !queue.empty();
                });

                if (stop && queue.empty()) {
                    return;
                }

                task = std::move(queue.front());
                queue.pop();
            }
            task();
        }
    }

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop{};
};

int main() {
    ThreadPool pool{4};

    for (size_t i{}; i < 10; ++i) {
        pool.submit([i]() {
            std::cout << std::format(
                "Task {} in thread {}\n",
                i,
                std::this_thread::get_id()
            );
        });
    }

    return 0;
}
```

```
Task 0 in thread 18416
Task 1 in thread 29136
Task 3 in thread 5144
Task 6 in thread 5144
Task 4 in thread 29136
Task 8 in thread 29136
Task 2 in thread 18416
Task 5 in thread 11812
Task 7 in thread 5144
Task 9 in thread 29136
```

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <format>
#include <future>

class ThreadPool {

public:
    explicit ThreadPool(const size_t num_threads) {
        for (size_t i{}; i < num_threads; ++i) {
            workers.emplace_back([this](){
                loop();
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock lock{mtx};
            stop = true;
        }
        cv.notify_all();

        for (auto &worker: workers) {
            worker.join();
        }
    }

    template<typename C>
    std::future<std::invoke_result_t<C>> submit(C&& callable) {
        using R = std::invoke_result_t<C>;
        auto task = std::make_shared<std::packaged_task<R()>>(
            std::forward<C>(callable)
        );

        auto future = task->get_future();
        {
            std::unique_lock lock{mtx};
            queue.push([task]() {
                (*task)();
            });
        }

        cv.notify_one();

        return future;
    }

private:
    void loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock lock{mtx};
                cv.wait(lock, [this]() {
                    return stop || !queue.empty();
                });

                if (stop && queue.empty()) {
                    return;
                }

                task = std::move(queue.front());
                queue.pop();
            }
            task();
        }
    }

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop{};
};

int main() {
    ThreadPool pool{4};

    std::vector<std::future<int>> futures;
    for (int i{}; i < 10; ++i) {
        futures.emplace_back(pool.submit([i]()-> int {
            std::cout << std::format(
                "Task {} in thread {}\n",
                i,
                std::this_thread::get_id()
            );

            return i*i;
        }));
    }

    for (auto &&future: futures) {
        std::cout << std::format("! {}\n", future.get());
    }

    return 0;
}
```

```
Task 0 in thread 18336
Task 3 in thread 18336
Task 1 in thread 19232
Task 5 in thread 15704
Task 4 in thread 18336
Task 7 in thread 15704
Task 9 in thread 15704
Task 6 in thread 19232
! 0
! 1
Task 8 in thread 18336
Task 2 in thread 27100
! 4
! 9
! 16
! 25
! 36
! 49
! 64
! 81
```

---
---
---
---
### Темы:
- `std::jthread` (C++20) — автоматический `join`
- `std::stop_token`, `std::stop_source` (C++20) — безопасное завершение
- Lock-free структуры данных (на базе `std::atomic`)
- Работа с GUI или сетевыми серверами в многопоточной среде
- Профилирование и отладка многопоточных программ

### Практика:
```cpp
// Написать простой пул потоков с очередью задач
```

---

## 🛠️ Инструменты и среды

| Инструмент | Для чего |
|----------|---------|
| **g++ / clang++ с `-pthread`** | Компиляция многопоточных программ |
| **Valgrind + Helgrind/DRD** | Поиск race conditions |
| **GCC/Clang с `-fsanitize=thread`** | ThreadSanitizer — лучший выбор |
| **IDE: CLion, VS Code, Visual Studio** | Отладка потоков |

---

## 📚 Рекомендуемые источники

### Книги:
- **"C++ Concurrency in Action"** — *Anthony Williams* (лучшая книга по теме)
- **"Effective Modern C++"** — *Scott Meyers* (разделы про concurrency)

### Онлайн:
- [https://en.cppreference.com](https://en.cppreference.com) — официальная документация
- [https://www.modernescpp.com](https://www.modernescpp.com) — отличные статьи по concurrency
- YouTube: поиск по "C++ threads tutorial"

---

## ✅ Советы по обучению

| Совет | Почему |
|------|--------|
| Пишите код каждый день | Многопоточность требует практики |
| Используйте ThreadSanitizer | Находит ошибки, которые вы не увидите сами |
| Начинайте с простого | Не бросайтесь сразу в lock-free программирование |
| Тестируйте на разных платформах | Поведение может отличаться (Linux vs Windows) |
| Избегайте глобальных переменных | Они усложняют тестирование |

---

## 🎯 Финальный проект (по окончании курса)

> **Создать HTTP-сервер (упрощённый), который:**
> - Обрабатывает запросы в отдельных потоках
> - Имеет thread-safe кэш
> - Использует пул потоков
> - Поддерживает асинхронные операции

---

Если хочешь, могу:
- Прислать пошаговые уроки с примерами
- Подготовить тесты по каждой теме
- Показать, как отлаживать deadlock
- Составить таблицу совместимости (C++11, 17, 20)

📌 Просто скажи: "Да, хочу подробный урок по [тема]"!

Удачи в изучении многопоточности! 💪