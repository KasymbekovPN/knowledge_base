---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::remove_reference_t<T>` из заголовка `<type_traits>` удаляет ссылку (`&` или `&&`) из типа `T`.

💡 Это **незаменимый инструмент** при работе с шаблонами, особенно с универсальными ссылками (`T&&`).

```cpp
#include <iostream>
#include <type_traits>

template<typename T, typename R>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int&, int>();
    test<int&&, int>();
    test<const char&, const char>();
    test<double, double>();

    test<int*&, int*>();
    test<std::string&&, std::string>();

    return 0;
}

template<typename T, typename R>
void test() {
    std::cout
        << std::is_same_v<std::remove_reference_t<T>, R>
        << std::endl;
}
```

```
true
true
true
true
true
true
```
