---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_fundamental_v<T>` из заголовка `<type_traits>` проверяет, является ли тип **фундаментальным (fundamental)** — то есть один из базовых встроенных типов C++.

> 🔧 Подключается через:
```cpp
#include <type_traits>
```

### Что считается "фундаментальным" типом?

`std::is_fundamental_v<T>` возвращает `true`, если `T` — это:

✅ Целочисленные типы:
- `bool`, `char`, `int`, `long`, `unsigned char`, и т.д.

✅ Числа с плавающей точкой:
- `float`, `double`, `long double`

✅ `void`

✅ `std::nullptr_t` (начиная с C++14)
### Не фундаментальные типы:
- Указатели (`int*`)
- Массивы (`int[5]`)
- Классы, структуры
- Перечисления (`enum`)
- Функции

```cpp
#include <iostream>
#include <type_traits>

struct SomeStruct{};

template<typename T>
void test(const T&);

int main() {
    test(42);
    test(3.14);
    test(true);
    test(SomeStruct{});

    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (std::is_fundamental_v<T>) {
        std::cout << "F: " << _value << std::endl;
    } else {
        std::cout << "Other" << std::endl;
    }
}
```

```
F: 42
F: 3.14
F: 1
Other
```

