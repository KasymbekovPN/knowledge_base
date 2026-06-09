---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::unsigned_integral<T>`** из заголовка `<concepts>` проверяет, что тип является **беззнаковым целочисленным (unsigned integer)**.

## 📘 Что означает `std::unsigned_integral<T>`?

Тип `T` удовлетворяет концепту, если он:
- Является целым (`std::integral<T>`),
- И его значения всегда неотрицательные (беззнаковый).

✅ Включает:
- `unsigned int`, `unsigned long`, `unsigned short`
- `size_t`, `uintptr_t`
- `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`
- `std::byte` (начиная с C++17)

❌ Не включает:
- `int`, `long`, `short` (знаковые)
- `char`, `signed char`, `bool`
- `float`, `double`, указатели

### Важные моменты

| Тип | Удовлетворяет? |
|-----|----------------|
| `unsigned int` | ✅ Да |
| `size_t` | ✅ Да (обычно `unsigned long`) |
| `uint32_t` | ✅ Да |
| `int` | ❌ Нет |
| `long long` | ❌ Нет |
| `char` | ❌ Зависит от реализации — лучше не использовать |
| `std::byte` | ✅ Да (C++17+) |

> 💡 Используйте `uint32_t`, `size_t` и т.д. — они однозначны.

---

### Лучшая практика

| Совет                                                                                   | Почему                              |
| --------------------------------------------------------------------------------------- | ----------------------------------- |
| Используйте `std::unsigned_integral` вместо `std::is_unsigned_v<T> && std::integral<T>` | Читаемее                            |
| Документируйте, где ожидаются только положительные значения                             | Например: размер, счётчик, индекс   |
| Избегайте `int` как счётчика                                                            | Может переполниться                 |
| Предпочитайте `size_t` или `std::uint_fast32_t` в критичных местах                      | Производительность и портабельность |


```cpp
#include <iostream>
#include <concepts>

template<std::unsigned_integral T>
void test(T);

int main(int argc, char const *argv[]) {
    test(42u);
    test(1000UL);
    test(size_t{123});
    test(uint32_t{999});

    // test(-5); // Error
    // test(3.14159); // Error

    return 0;
}

template<std::unsigned_integral T>
void test(T _input) {
    std::cout << "Input: " << _input << std::endl;
}
```

```
Input: 42
Input: 1000
Input: 123
Input: 999
```
