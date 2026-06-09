---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Проверяет, является ли тип `T` **типом с плавающей точкой**:

- `float`
- `double`
- `long double`

> 🔧 Доступен с C++17 как переменная-шаблон.  
> Аналог: `std::is_floating_point<T>::value`

```cpp
#include <type_traits>
```

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void test(const T&);

int main() {
    test(3.14159);
    test(42);
    test(2.5f);
    test('A');
    test(true);

    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (std::is_floating_point_v<T>) {
        std::cout << "FP: " << _value << std::endl;
    } else {
        std::cout << "Other: " << _value << std::endl;
    }
}
```

```
FP: 3.14159
Other: 42  
FP: 2.5
Other: A
Other: 1
```
