---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::conditional_t<Condition, T, U>`** из заголовка `<type_traits>` выбирает один из двух типов в зависимости от булева условия на этапе компиляции.

### Что делает `std::conditional_t<C, T, U>`?

Он работает как тернарный оператор, но для **типов**:

| Условие | Результат |
|--------|-----------|
| `C == true` → `T` |
| `C == false` → `U` |

✅ Это основа метапрограммирования: выбор типа по условию без шаблонной специализации.

```cpp
#include <iostream>
#include <type_traits>

#define TEST(T, U) \
    std::cout \
        << #T " & " #U ": "\
        << std::boolalpha \
        << std::is_same_v<T, U> \
        << std::noboolalpha \
        << std::endl;

using UType = std::conditional_t<sizeof(void*) == 8, uint64_t, uint32_t>;

int main() {
    TEST(uint32_t, UType);
    TEST(uint64_t, UType);
    std::cout << typeid(UType).name() << std::endl;

    return 0;
}
```

```
uint32_t & UType: false
uint64_t & UType: true
unsigned __int64
```
