---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

Когда создаётся поток:

```cpp
std::thread t(work);
```

происходят две независимые вещи:
1. Объект `t` в текущем потоке.
2. Новый поток выполнения в ОС.

```text
main thread
    |
    +---- std::thread t
                |
                +---- worker thread
```

Кто будет отвечать за завершение worker-потока.
Для этого существуют два варианта:

```cpp
t.join();
```

или

```cpp
t.detach();
```

# join()

`join()` означает:
> Подождать завершения потока и забрать его ресурсы.

```cpp
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Main started" << std::endl;

    std::thread t{[]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Thread done" << std::endl;
    }};
    t.join();

    std::cout << "Main finished" << std::endl;

    return 0;
}
```

```
Main started
Thread done
Main finished
```

Что происходит:

```text
main
 |
 | create thread
 v
worker running
 |
 | join()
 |
main waits
 |
worker finished
 |
main continues
```

После `t.join();` объект `t` больше не связан с потоком. Проверка `t.joinable()` вернёт `false`.

# detach()

`detach()` означает:
> Отделить поток от объекта std::thread.

```cpp
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Main started" << std::endl;

    std::thread t{[]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Thread done" << std::endl;
    }};
    t.detach();

    std::cout << "Main finished" << std::endl;

    return 0;
}
```

```
Main started
Main finished
```

После `t.detach();` объект `t` больше не управляет потоком.

```text
main thread
    |
    +---- detached thread
```

ОС сама освободит ресурсы потока после его завершения.

# Визуально

## join

```text
main
 |
 +----- worker
 |
 wait
 |
 worker finished
 |
 continue
```

## detach

```text
main
 |
 +----- worker
 |
 continue immediately
 |
 main may finish first
```

# Почему многие избегают detach

После:

```cpp
t.detach();
```

ты теряешь возможность:
- дождаться завершения;
- узнать статус;
- обработать исключения;
- синхронизировать завершение.

Поток становится "сам по себе".

# Что будет если не вызвать ни join(), ни detach()

Самая распространённая ошибка новичков.

```cpp
int main() {
    std::thread t([]{});
}
```

При уничтожении `t`:

```cpp
~thread()
```

обнаруживается:

```cpp
t.joinable() == true
```

и вызывается:

```cpp
std::terminate();
```

Программа аварийно завершится.

# Правило

Для любого `std::thread` должен быть выполнен ровно один из вариантов:

```cpp
t.join();
```

или

```cpp
t.detach();
```

до уничтожения объекта.

# Современный C++20: std::jthread

Поэтому появился:

```cpp
std::jthread t(work);
```

Его деструктор автоматически делает:

```cpp
join();
```

Поэтому такой код безопасен:

```cpp
{
    std::jthread t(work);
}
```

Поток автоматически завершится перед выходом из области видимости.

# Когда использовать

### `join()`

Если результат работы потока важен.

```cpp
download file
calculate result
write database
```

Это основной и наиболее безопасный вариант.

### `detach()`

Если нужен полностью фоновый поток.

Например:

```cpp
fire-and-forget logging
background telemetry
watchdog
```

Но нужно гарантировать, что все используемые данные переживут поток.

# Практическое правило

Для нового кода:

```cpp
std::jthread
```

предпочтительнее `std::thread`.

Если используешь `std::thread`, по умолчанию выбирай:

```cpp
join()
```

а `detach()` применяй только тогда, когда действительно нужен независимый фоновый поток и ты контролируешь время жизни всех используемых объектов.
