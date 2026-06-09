---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::add_const_t<T>` из заголовка `<type_traits>` добавляет квалификатор `const` к типу `T`.

⚠️ Важно:  
- Для **указателей** `add_const_t` делает **указатель константным**, а не данные!
- Чтобы сделать **данные константными**, нужно использовать другие подходы.

```cpp
#include <iostream>
#include <type_traits>

template<typename T, typename C>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int, const int>();
    test<const int, const int>();
    test<double, const double>();

    test<int*, int* const>();
    test<char*, char* const>();

    test<const char*, const char*>();

    return 0;
}

template<typename T, typename C>
void test() {
    std::cout
        << std::is_same_v<std::add_const_t<T>, C>
        << std::endl;
}
```


```
true
true
true
true
true
false
```
