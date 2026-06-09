---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::signed_integral<T>`** из заголовка `<concepts>` проверяет, что тип является **знаковым целочисленным (signed integer)**.

### Что означает `std::signed_integral<T>`?

Тип `T` удовлетворяет концепту, если он:
- Является целым (`std::integral<T>`),
- И его значение может быть отрицательным (т.е. не беззнаковый).

✅ Включает:
- `int`, `short`, `long`, `long long`
- `signed char` (но не `char` — зависит от реализации!)
- `wchar_t` (если знаковый)
- Перечисления с базовым знаковым типом

❌ Не включает:
- `unsigned int`, `size_t`, `uint32_t`
- `bool`, `char16_t`, `char32_t`, `char8_t`
- `float`, `double`, указатели

### Важные моменты

| Тип | Удовлетворяет `std::signed_integral`? |
|-----|--------------------------------------|
| `int` | ✅ Да |
| `long` | ✅ Да |
| `short` | ✅ Да |
| `signed char` | ✅ Да |
| `char` | ❌ Зависит от компилятора — не гарантировано! |
| `unsigned int` | ❌ Нет |
| `size_t` | ❌ Нет |
| `int8_t`, `int16_t`, `int32_t`, `int64_t` | ✅ Да |
| `std::make_signed_t<unsigned int>` | ✅ Да — результат `int` |

> 💡 Чтобы быть уверенным, используйте явные типы: `int32_t`, `int64_t`.

### Лучшая практика

| Совет                                                                                    | Почему                       |
| ---------------------------------------------------------------------------------------- | ---------------------------- |
| Используйте `std::signed_integral` вместо `std::is_signed_v<T> && std::is_integral_v<T>` | Читаемее и безопаснее        |
| Избегайте `char` в таких контекстах                                                      | Его знаковость не определена |
| Документируйте требования к параметрам                                                   | Особенно в API               |
| Предпочитайте фиксированные размеры (`int32_t`) в критичных местах                       | Портабельность               |

```cpp
#include <iostream>
#include <concepts>

template<std::signed_integral T>
void test(T);

int main() {
    test(42);
    test('A');
    // test(true); // Error
    // test(100U); // Error
    // test(std::string("hello")); // Error    

    return 0;
}

template<std::signed_integral T>
void test(T _input) {
    std::cout << "Value: " << _input << std::endl;
}
```

```
Value: 42
Value: A
```
