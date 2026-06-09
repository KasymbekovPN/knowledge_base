---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::conjunction_v<B1, B2, ...>`** из заголовка `<type_traits>` вычисляет логическое **И (AND)** для списка булевых типов-предикатов.

### Что делает `std::conjunction_v<...>`?

Он возвращает `true`, если **все** переданные условия — `true`.

✅ Аналог:
```cpp
B1::value && B2::value && ...
```
но работает с **типами**, и останавливается при первом `false` (как ленивый `&&`).

```cpp
#include <iostream>

template<typename T>
std::enable_if_t<
    std::conjunction_v<
        std::is_integral<T>,
        std::negation<
            std::is_const<T>
        >
    >
>
test(T);

int main() {
    test(42);
    // test(12.34); // Error

    return 0;
}

template<typename T>
std::enable_if_t<
    std::conjunction_v<
        std::is_integral<T>,
        std::negation<
            std::is_const<T>
        >
    >
>
test(T _input) {
    std::cout
        << "Value: " << _input << std::endl
        << "Type: " << typeid(_input).name()
        << std::endl;
}
```

```
Value: 42
Type: int
```

```
error: no matching function for call to 'test'
```
