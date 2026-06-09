---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::disjunction_v<B1, B2, ...>`** из заголовка `<type_traits>` вычисляет логическое **ИЛИ (OR)** для списка булевых типов-предикатов.
### Что делает `std::disjunction_v<...>`?

Он возвращает `true`, если **хотя бы один** из переданных условий — `true`.

✅ Аналог:
```cpp
B1::value || B2::value || ...
```
но работает на этапе компиляции и поддерживает **ленивое вычисление** (как обычный `||`).

## ⚠️ Важные моменты

| Особенность | Пояснение |
|------------|----------|
| Ленивое вычисление | Если первый аргумент `true`, остальные не проверяются |
| Пустой список (`<>`) → `false` | Нейтральный элемент для OR |
| Требует типы с `::value` | Например: `std::is_integral<T>`, `std::true_type` |

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
std::enable_if_t<
    std::disjunction_v<
        std::is_same<T, int>,
        std::is_same<T, float>,
        std::is_same<T, double>
    >
>
test(T&&);

int main() {
    test(42);
    test(12.34);
    test(2.72f);
    // test("hello"); // Error

    return 0;
}

template<typename T>
std::enable_if_t<
    std::disjunction_v<
        std::is_same<T, int>,
        std::is_same<T, float>,
        std::is_same<T, double>
    >
>
test(T&& _input) {
    std::cout << "V: " << _input << std::endl;
}
```

```
V: 42
V: 12.34
V: 2.72
```

```
error: no matching function for call to 'test'
```
