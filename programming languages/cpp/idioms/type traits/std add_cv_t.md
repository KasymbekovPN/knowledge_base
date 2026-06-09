---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::add_cv_t<T>` из заголовка `<type_traits>` добавляет **оба квалификатора** к типу `T`:
- `const`
- `volatile`

> ⚠️ Для **указателей** `add_cv_t` делает **сам указатель `const volatile`**, а не данные, на которые он указывает.

```cpp
#include <iostream>
#include <type_traits>

template<typename T, typename C>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int, const volatile int>();
    test<double, const volatile double>();
    test<int*, int* const volatile>();
    test<char*, char* const volatile>();

    test<const int, const volatile int>();
    test<const volatile int, const volatile int>();

    return 0;
}

template<typename T, typename C>
void test() {
    std::cout
        << std::is_same_v<std::add_cv_t<T>, C>
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
