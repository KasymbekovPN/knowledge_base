---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

Механизм синхронизации, который позволяет одному потоку **ждать**, пока другой поток не выполнит условие и не **уведомит** его.

Требует `std::unique_lock` (не `lock_guard`) — потому что мьютекс должен временно освобождаться во время ожидания.

### Базовые методы

```cpp
cv.wait(lock, predicate);  // ждать пока predicate не вернёт true
cv.notify_one();           // разбудить один ждущий поток
cv.notify_all();           // разбудить все ждущие потоки
```

### Когда использовать

| Задача                    | Инструмент                                     |
| ------------------------- | ---------------------------------------------- |
| Защита данных от гонки    | `mutex` + `lock_guard`                         |
| Ожидание условия          | `mutex` + `condition_variable` + `unique_lock` |
| Однократная инициализация | `call_once`                                    |
| Результат из потока       | `std::future` / `std::promise`                 |

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

constexpr size_t SIZE{5};

std::mutex mtx;
std::condition_variable cv;
std::queue<int> buffer;

void run_producer() {
    for (size_t i{}; i < SIZE; ++i) {
        std::unique_lock lock{mtx};
        buffer.push(i);
        std::cout << "produced: " << i << std::endl;
    }
    cv.notify_one();
}

void run_consumer() {
    for (size_t i{}; i < SIZE; ++i) {
        std::unique_lock lock{mtx};
        cv.wait(lock, []() { return !buffer.empty(); });
        int value = buffer.front();
        buffer.pop();
        std::cout << "consumed: " << value << std::endl;
    }
}

int main() {
    std::thread t_producer{run_producer};
    std::thread t_consumer{run_consumer};

    t_producer.join();
    t_consumer.join();

    return 0;
}
```

```
produced: 0
produced: 1
produced: 2
produced: 3
produced: 4
consumed: 0
consumed: 1
consumed: 2
consumed: 3
consumed: 4
```

### Как работает wait

```cpp
cv.wait(lock, predicate);
```

Это эквивалентно:

```cpp
while (!predicate()) {
    lock.unlock();   // освободить мьютекс — другие потоки могут работать
    // спать до notify
    lock.lock();     // снова захватить перед проверкой
}
```

Predicate защищает от **spurious wakeup** — ложных пробуждений, которые может генерировать ОС. Без него поток мог бы продолжить работу без реальных данных.

### notify_one vs notify_all

```cpp
cv.notify_one(); // разбудить одного — для очереди задач (один забирает одну задачу)
cv.notify_all(); // разбудить всех  — для broadcast (например, сигнал завершения)
```

