---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::add_volatile_t<T>` из заголовка `<type_traits>` добавляет квалификатор `volatile` к типу `T`.

### Что делает `std::add_volatile_t<T>`?

⚠️ Важно:
- Для **указателей** `add_volatile_t` делает **сам указатель `volatile`**, а не данные!
- Чтобы сделать **данные `volatile`**, нужно использовать специализацию.

```cpp
#include <iostream>
#include <type_traits>

template<typename T, typename C>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int, volatile int>();
    test<double, volatile double>();
    test<volatile int, volatile int>();

    test<int*, int* volatile>();
    test<char*, char* volatile>();

    return 0;
}

template<typename T, typename C>
void test() {
    std::cout
        << std::is_same_v<std::add_volatile_t<T>, C>
        << std::endl;
}
```

```
true
true
true
true
true
```
