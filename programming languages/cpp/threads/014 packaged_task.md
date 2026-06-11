---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

### std::packaged_task

Обёртка над вызываемым объектом (функцией, лямбдой), которая связывает его с `std::future`. Позволяет **отделить создание задачи от её запуска**.

### Отличие от async и promise

| |`async`|`promise`|`packaged_task`|
|---|---|---|---|
|Кто запускает|runtime|вручную через `set_value`|вручную через `operator()`|
|Возврат значения|из функции|`set_value()` явно|из функции автоматически|
|Гибкость|низкая|высокая|средняя|

### Когда использовать

- Нужно создать задачу сейчас, запустить позже
- Реализация пула потоков или очереди задач
- Нужен `future` без немедленного запуска (в отличие от `async`)

`packaged_task` — **non-copyable**, передаётся только через `std::move`.

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <queue>
#include <functional>

std::queue<std::packaged_task<int()>> task_queue;

int main() {
    std::packaged_task<int()> fail_task{[]() -> int {
        throw std::runtime_error("sth fail");
    }};
    auto&& fail_future = fail_task.get_future();
    fail_task();

    try {
        fail_future.get();
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    std::packaged_task<int()> t1{[]() { return 1 + 1; }};
    std::packaged_task<int()> t2{[]() { return 10 * 5; }};

    auto&& f1 = t1.get_future();
    auto&& f2 = t2.get_future();

    task_queue.push(std::move(t1));
    task_queue.push(std::move(t2));

    std::thread worker{[&]() {
        while (!task_queue.empty()) {
            auto task = std::move(task_queue.front());
            task_queue.pop();
            task();
        }
    }};
    worker.join();

    std::cout << f1.get() << "\n";
    std::cout << f2.get() << "\n";

    return 0;
}
```

```
sth fail
2
50
```
