---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

`std::thread` — это класс стандартной библиотеки C++, представляющий **поток выполнения (execution thread)**.

Он позволяет запускать функцию параллельно основному потоку программы.

Пример:

```cpp
#include <iostream>
#include <thread>

void start_worker() {
    std::cout << "Worker executed" << std::endl;
}

int main() {
    std::thread t{start_worker};
    t.join();

    return 0;
}
```

```
Worker executed
```

Что происходит:

```text
main()
 ├─ основной поток
 └─ std::thread t(work)
       └─ второй поток выполняет work()
```

Основные методы:

|Метод|Назначение|
|---|---|
|`join()`|дождаться завершения потока|
|`detach()`|отделить поток, он продолжит работу самостоятельно|
|`joinable()`|проверить, связан ли объект с потоком|
|`get_id()`|получить идентификатор потока|
Важное правило:

```cpp
std::thread t(work);
```

Перед уничтожением объекта `t` необходимо вызвать либо:

```cpp
t.join();
```

либо

```cpp
t.detach();
```

Иначе программа завершится через `std::terminate()`.

Кратко: **`std::thread` — это стандартный механизм C++ для запуска и управления параллельными потоками выполнения.**
