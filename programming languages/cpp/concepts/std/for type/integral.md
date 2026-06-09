---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::integral`** из заголовка `<concepts>` проверяет, что тип является **целочисленным (integer)**.

### Что означает `std::integral<T>`?

Тип `T` удовлетворяет концепту `std::integral`, если он:
- Является целочисленным типом: `int`, `short`, `long`, `char`, `bool`, `wchar_t`, `char8_t`, `char16_t`, `char32_t`,
- Или это **беззнаковый/знаковый вариант** (`signed char`, `unsigned long`),
- Не включает: `float`, `double`, указатели, классы.

✅ Это современная замена `std::is_integral_v<T>` — но с более читаемым и безопасным синтаксисом.

### Важные моменты

| Тип | Удовлетворяет `std::integral`? |
|-----|-------------------------------|
| `int` | ✅ Да |
| `unsigned long` | ✅ Да |
| `char` | ✅ Да |
| `bool` | ✅ Да |
| `float` | ❌ Нет |
| `double` | ❌ Нет |
| `int*` | ❌ Нет |
| `enum class Color : int` | ❌ Нет! Сам enum — не integral, даже если базовый тип — int |

> 💡 Чтобы проверить базовый тип перечисления — используйте:
```cpp
std::is_same_v<std::underlying_type_t<Color>, int>
```

### Лучшая практика

| Совет | Почему |
|------|--------|
| Используйте `std::integral` вместо `std::is_integral_v<T>` в C++20+ | Более читаемо |
| Документируйте требования к типам | Особенно в библиотечном коде |
| Предпочитайте его при работе с индексами, размерами, битовыми операциями | Логически обоснованно |
| Избегайте в местах, где нужен любой арифметический тип | Используйте `std::arithmetic` тогда |

```cpp
#include <iostream>
#include <concepts>

template<std::integral T>
void test(T);

int main() {
    test(42);
    test('A');
    test(true);
    test(100LL);
    // test(std::string("hello")); // Error    

    return 0;
}

template<std::integral T>
void test(T _input) {
    std::cout << "Value: " << _input << std::endl;
}
```

```
Value: 42
Value: A
Value: 1
Value: 100
```
