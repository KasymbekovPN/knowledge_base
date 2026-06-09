---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::make_signed_t<T>` из заголовка `<type_traits>` преобразует **беззнаковый целый тип** в соответствующий **знаковый**.

### Что делает `std::make_signed_t<T>`?

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

    test<unsigned int, int>();
    test<unsigned long, long>();
    test<uint8_t, int8_t>();
    test<uint16_t, int16_t>();
    test<uint32_t, int32_t>();
    test<uint64_t, int64_t>();

    return 0;
}

template<typename T, typename R>
void test() {
    std::cout
        << std::is_same_v<std::make_signed_t<T>, R>
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
