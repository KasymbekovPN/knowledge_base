---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

`std::thread` может запускать любой **callable object**: функцию, лямбду, функтор, метод класса и т.д.

### Что принимает конструктор `std::thread`

Упрощённо:

```cpp
template<class F, class... Args>
thread(F&& f, Args&&... args);
```

Внутри происходит примерно:

```cpp
std::invoke(f, args...);
```

Поэтому всё, что удовлетворяет концепту `std::invocable`, можно запускать в отдельном потоке:

- функции;
- лямбды;
- функторы;
- указатели на методы;
- указатели на данные-члены;
- объекты `std::bind`.

### Обычная функция

```cpp
#include <iostream>
#include <thread>

void do_work() {
    std::cout << "do_work executed." << std::endl;
}

int main() {
    std::thread t(do_work);
    t.join();

    return 0;
}
```

```
do_work executed.
```

### Лямбда

```cpp
#include <iostream>
#include <thread>

int main() {
    std::thread t0([]() {
        std::cout << "thread executed" << std::endl;
    });

    std::thread t1([](const int _input) {
        std::cout << "thread executed :: " << _input << std::endl;
    }, 42);

    t0.join();
    t1.join();

    return 0;
}
```

```
thread executed
thread executed :: 42
```

### Функтор

```cpp
#include <iostream>
#include <thread>

struct Worker {
    const std::string id;
    int value{};
    int result{};

    Worker(const int _value, const std::string _id):
        value{_value},
        id{_id} {}

    void operator()() {
        result = value * value;
        std::cout << "[" << id << "] inner result: " << result << std::endl;
    }

    int getResult() const {
        return result;
    }
};

struct MWorker {
    const std::string id;
    int value{};
    int result{};

    MWorker(const int _value, const std::string _id):
        value{_value},
        id{_id} {}

    MWorker(const MWorker&) = delete;
    MWorker(MWorker&&) = default;

    void operator()() {
        result = value * value;
        std::cout << "[" << id << "] inner result: " << result << std::endl;
    }

    int getResult() const {
        return result;
    }
};

int main() {
    Worker w00(42, "w00");
    std::thread t00 {w00};
    t00.join();
    std::cout << "w00.result " << w00.result << std::endl;

    std::thread t01{Worker{43, "w01"}};
    t01.join();

    std::thread t02{MWorker{44, "w02"}};
    t02.join();

    MWorker w03{45, "w03"};
    // std::thread t03 {w03}; // Error
    std::thread t03 {std::move(w03)};
    t03.join();
    std::cout << "w03.result " << w03.result << std::endl;

    return 0;
}
```

__При передаче готового объекта__
- передаётся существующий объект;
- обычно копируется;
- объект остаётся доступен после запуска потока.

__При использовании временного объекта__
- передаётся временный объект;
- обычно перемещается;
- после передачи к объекту доступа нет.

### Метод объекта

> Здесь передача объекта по указателю и по `std::reference_wrapper<...>` эквивалентно.

Для `std::thread t{&Worker::run, &w};` вызывается `std::invoke(&Worker::run, &w);`

Для `std::thread t1{&Worker::run, std::ref(w)};` вызывается `std::invoke(&Worker::run, std::ref(w));`

```cpp
#include <iostream>
#include <thread>

struct Worker {
    int data{};

    void run() {
        data++;
    }
};

int main() {
    Worker w;
    std::thread t{&Worker::run, &w};
    t.join();
    std::cout << "w.data " << w.data << std::endl;

    std::thread t1{&Worker::run, std::ref(w)};
    t1.join();

    std::cout << "w.data " << w.data << std::endl;  

    return 0;
}
```

```
w.data 1
w.data 2
```

### Константный метод объекта

```cpp
#include <iostream>
#include <thread>

struct Worker {
    void run() const {
        std::cout << "Worker::run" << std::endl;
    }
};

int main() {
    Worker w;
    std::thread t{&Worker::run, &w};
    t.join();

    return 0;
}
```

```
Worker::run
```

### Функция с аргументами через копию

```cpp
#include <iostream>
#include <thread>

void print_sum(const int x, const int y) {
    std::cout
        << x << " + " << y
        << " = " << (x + y)
        << std::endl;
}

int main() {
    std::thread t{print_sum, 1, 42};
    t.join();

    return 0;
}
```

```
1 + 42 = 43
```

### Функция с аргументами через ссылку

```cpp
#include <iostream>
#include <thread>

void inc(int& _input) {
    _input++;
}

int main() {
    int value{0};
    std::thread t{inc, std::ref(value)};
    t.join();

    std::cout << "value: " << value << std::endl;

    return 0;
}
```

```
value: 1
```

### Через `std::bind`

`std::bind` позволяет **заранее связать (bind)** функцию с частью или всеми её аргументами и получить новый вызываемый объект (callable).

В контексте `std::thread` это означает, что можно подготовить задачу заранее, а затем передать её в поток.

Исторически (до появления лямбд в C++11) это был основной способ создавать адаптеры функций.

В контексте `std::thread`:

```
std::bind(...)
```

- превращает функцию + аргументы в готовый callable-объект;
- позволяет частично фиксировать аргументы;
- удобно для хранения и повторного использования задач.

Но в современном C++ (C++14/17/20/23) в большинстве случаев вместо `std::bind` предпочитают лямбды: они проще читаются, лучше поддерживаются IDE и обычно дают более понятные сообщения об ошибках.

# Когда `bind` всё ещё полезен

Иногда удобно передать уже готовый callable:

```cpp
auto task = std::bind(&Worker::run, &worker, 42);
queue.push(task);
thread_pool.submit(task);
std::thread t(task);
```

То есть однажды собрать вызов и потом многократно его использовать.

```cpp
#include <iostream>
#include <thread>
#include <functional>

void print_sum(const int a, const int b) {
    std::cout
        << a << " + "  << b
        << " = " << a + b
        << std::endl;
}

int main() {
    auto&& add10 = std::bind(print_sum, 10, std::placeholders::_1);
    add10(42);

    auto&& task = std::bind(print_sum, 1, 2);
    std::thread t{task};
    t.join();

    return 0;
}
```

```
10 + 42 = 52
1 + 2 = 3
```
