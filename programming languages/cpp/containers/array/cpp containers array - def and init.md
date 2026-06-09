---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - array
---
[[_cpp containers - array|<=]]

В C++ `std::array` — это контейнер, который представляет собой статический массив фиксированного размера. В отличие от `std::vector`, размер `std::array` должен быть известен на этапе компиляции и не может изменяться во время выполнения программы.

#### Инициализация по умолчанию

Элементы массива инициализируются значениями по умолчанию для указанного типа.

```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 5> array {};
    for (auto &&item:  array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

```
0 0 0 0 0
```

#### Инициализация списком значений

Массив можно инициализировать с помощью списка инициализации.

```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 5> array {0, 1, 2, 3, 4};
    for (auto &&item:  array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

```
0 1 2 3 4 
```

#### Частичная инициализация

Если список инициализации короче размера массива, оставшиеся элементы инициализируются значением по умолчанию.

```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 5> array {0, 1, 2};
    for (auto &&item:  array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

```
0 1 2 0 0 
```

#### Инициализация с использованием `fill`

Метод `fill` заполняет все элементы массива указанным значением.

```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 5> array;
    array.fill(42);

    for (auto &&item:  array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

```
42 42 42 42 42
```

#### Инициализация с использованием `std::to_array` (C++20)

Начиная с C++20, можно использовать функцию `std::to_array` для создания `std::array` из встроенного массива или списка инициализации.

```cpp
#include <iostream>
#include <array>

int main() {
    auto array = std::to_array({111, 222, 333});

    for (auto &&item:  array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

```
111 222 333
```
