---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::make_unsigned_t<T>` из заголовка `<type_traits>` преобразует **беззнаковый целый тип** в соответствующий **знаковый**.

### Что делает `std::make_unsigned_t<T>`?

⚠️ Требования:
- `T` должен быть **целочисленным типом**,
- Должен существовать знаковый аналог того же размера.

❌ Не работает для:
- `bool`,
- `char` (неоднозначно: signed или unsigned),
- `float`, `double`,
- Указателей, классов.

```cpp
#include <iostream>
#include <type_traits>

template<typename T, typename R>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int, unsigned int>();
    test<long, unsigned long>();
    test<int8_t, uint8_t>();
    test<int16_t, uint16_t>();
    test<int32_t, uint32_t>();
    test<int64_t, uint64_t>();

    return 0;
}

template<typename T, typename R>
void test() {
    std::cout
        << std::is_same_v<std::make_unsigned_t<T>, R>
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
