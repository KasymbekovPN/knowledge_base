---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

### std::promise

Пара к `std::future` — позволяет **вручную установить значение или исключение** из одного потока, которое получит другой через `future`.

`async` создаёт `promise` автоматически. `promise` нужен когда нужен явный контроль — например результат формируется не в одной функции.

### Связка promise → future

```cpp
std::promise<int> p;
std::future<int> f = p.get_future(); // получить future из promise

// в другом потоке
p.set_value(42);      // установить результат
// или
p.set_exception(std::make_exception_ptr(std::runtime_error("fail")));
```

### promise vs async

| |`async`|`promise`|
|---|---|---|
|Создание future|автоматически|вручную через `get_future()`|
|Установка значения|возврат из функции|`set_value()` явно|
|Гибкость|низкая|высокая|
|Использование|простые случаи|сложные pipeline, callback-и|

**Правило:** `async` когда результат — возврат функции. `promise` когда результат формируется в произвольный момент или передаётся через callback.

```cpp
#include <iostream>
#include <thread>
#include <future>

void compute(std::promise<int> _promise, int _value) {
    _promise.set_value(_value * _value);
}

void risky(std::promise<int> _promise) {
    try {
        throw std::runtime_error("sth went wrong");
    } catch(const std::exception& e) {
        _promise.set_exception(std::current_exception());
    }
}

void only_signal(std::promise<void> _promise) {
    _promise.set_value();
}

int main() {
    std::promise<int> p0;
    auto&& f0 = p0.get_future();
    std::thread t0{compute, std::move(p0), 10};
    t0.join();
    std::cout << "t0: " << f0.get() << std::endl;

    std::promise<int> p1;
    auto&& f1 = p1.get_future();
    std::thread t1{risky, std::move(p1)};
    t1.join();

    try {
        f1.get();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    std::promise<void> p2;
    auto&& f2 = p2.get_future();
    std::thread t2{only_signal, std::move(p2)};
    t2.join();
    f2.wait();
    std::cout << "Done" << std::endl;

    return 0;
}
```

```
t0: 100
sth went wrong
Done
```
