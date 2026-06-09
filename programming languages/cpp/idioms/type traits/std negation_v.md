---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::negation_v<T>`** из заголовка `<type_traits>` вычисляет логическое **НЕ (NOT)** для булевого типа-предиката.

### Что делает `std::negation_v<T>`?

Он возвращает:
- `true`, если `T::value == false`
- `false`, если `T::value == true`

✅ Это эквивалентно `!T::value`, но работает с типами и используется в метапрограммировании.

> 💡 `std::negation_v<T>` ≡ `!std::bool_constant<T>::value`

## ⚠️ Важные моменты

| Выражение | Результат |
|----------|-----------|
| `std::negation_v<std::true_type>` | `false` |
| `std::negation_v<std::false_type>` | `true` |
| `std::negation_v<std::is_fundamental<int>>` | `false` → int — фундаментальный |
| `std::negation_v<std::is_class<SomeClass>>` | `false` |

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
std::enable_if_t<
    std::negation_v<
        std::is_pointer<T>
    >
>
test(T);

int main() {
    test(42);
    // test("Hello"); // Error

    return 0;
}

template<typename T>
std::enable_if_t<
    std::negation_v<
        std::is_pointer<T>
    >
>
test(T _input) {
    std::cout << "V: " << _input << std::endl;
}
```

```
V: 42
```

```
error: no matching function for call to 'test'
```
