---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_arithmetic_v<T>` из заголовка `<type_traits>` проверяет, является ли тип **арифметическим** — то есть:

- Либо **целочисленным** (`int`, `char`, `bool`, `unsigned long` и т.д.),
- Либо **числом с плавающей точкой** (`float`, `double`, `long double`).

> 🔧 Подключается через:
```cpp
#include <type_traits>
```

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void test(const T&);

int main() {
    test(42);
    test(3.14);
    test('A');
    test(true);
    test(std::string("Hello"));

    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (std::is_arithmetic_v<T>) {
        std::cout << "ARI: " << _value << std::endl;
    } else {
        std::cout << "Other: " << _value << std::endl;
    }
}
```

```
ARI: 42
ARI: 3.14
ARI: A
ARI: 1
Other: Hello
```
