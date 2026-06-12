---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

Лямбды — это самый распространённый способ работы с `std::thread` в современном C++, потому что они позволяют удобно передавать код и данные в новый поток.

Лямбда:

```cpp
[] {
    std::cout << "Hello from thread\n";
}
```

компилируется примерно в такой функтор:

```cpp
struct Lambda {
    void operator()() const {
        std::cout << "Hello from thread\n";
    }
};
```

А поток фактически получает объект этого типа.

# Базовый пример

```cpp
#include <iostream>
#include <thread>

int main() {
    std::thread t{[]() {
        std::cout << "In the thread" << std::endl;
    }};
    t.join();

    return 0;
}
```

```
In the thread
```

# Захват переменных по копии

Главная причина использовать лямбды в потоках — возможность передавать данные через захват.

```cpp
#include <iostream>
#include <thread>

int main() {
    int x{1};
    int y{2};

    std::thread t0 {[x]() {
        std::cout << "x: " << x << std::endl;
    }};
    t0.join();

    std::thread t1 {[=]() {
        std::cout << "x: " << x << ", y: " << y << std::endl;
    }};
    t1.join();

    return 0;
}
```

```
x: 1
x: 1, y: 2
```

# Захват переменных по ссылке

```cpp
#include <iostream>
#include <thread>

int main() {
    int x{};
    int y{};

    std::thread t0 {[&x]() {
        x++;
    }};
    t0.join();

    std::thread t1 {[&]() {
        x++;
        y++;
    }};
    t1.join();

    std::cout << "x: " << x << ", y: " << y << std::endl;

    // UB
    // std::thread t2;
    // {
    //     int z{42};
    //     t2 = std::thread([&z]() {
    //         z++;
    //         std::cout << "z: " << z << std::endl;
    //     });
    // }
    // t2.join();

    return 0;
}
```

```
x: 2, y: 1
```

> После выхода из блока `z` уничтожается. Поток может попытаться обратиться к уже несуществующей переменной. Получаем неопределённое поведение.

# Передача move-only объектов

Одна из сильных сторон лямбд. Это очень часто используется в многопоточном коде.

```cpp
#include <iostream>
#include <thread>

int main() {
    auto&& ptr = std::make_unique<int>(42);

    // std::thread t0 {[ptr]() {}}; // Error
    std::thread t1 {[p = std::move(ptr)]() {
        std::cout << *p << std::endl;
    }};
    t1.join();

    return 0;
}
```

```
42
```

# mutable лямбды

По умолчанию захваченные по значению объекты внутри лямбды считаются `const`.

```cpp
#include <iostream>
#include <thread>

int main() {
    int x{};

    // Error
    // std::thread t0 {[x]() {
    //     ++x;
    // }};

    std::thread t1 {[x]() mutable {
        ++x;
        std::cout << x << std::endl;
    }};
    t1.join();

    return 0;
}
```

```
1
```

# Передача параметров через thread

```cpp
#include <iostream>
#include <thread>

int main() {
    std::thread t{
        [](const int a, const int b) {
            std::cout << a + b << std::endl;
        },
        10,
        20
    };
    t.join();

    return 0;
}
```

```
30
```

# Захват this

```cpp
#include <iostream>
#include <thread>

struct Worker {
    int value{42};

    void run0() {
        std::thread t{[this]() {
            std::cout << "run0 " << value << std::endl;
        }};
        t.join();
    }

    void run1() {
        std::thread([this]() {
            std::cout << "run1 " << value << std::endl;
        }).detach();
    }
};

int main() {
    Worker w0;
    w0.run0();

    {
        Worker w1;
        w1.run1();
    }

    return 0;
}
```

```
run0 42
```

> Если объект уничтожится раньше завершения потока, то `this` станет висячим указателем. Поэтому с `detach()` нужно быть особенно осторожным.
