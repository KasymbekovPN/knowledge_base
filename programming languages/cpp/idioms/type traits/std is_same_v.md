---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_same_v<T, U>` из заголовка `<type_traits>` проверяет, являются ли **два типа `T` и `U` одинаковыми**.

### Что делает `std::is_same_v<T, U>`?

Возвращает `true`, если:
- `T` и `U` — **точно один и тот же тип** (с учётом `const`, `volatile`, ссылок).

Возвращает `false` в любом другом случае, даже если типы "похожи".

```cpp
#include <iostream>
#include <type_traits>

#define TEST(T0, T1) \
    std::cout \
        << #T0 " == " #T1 ": " \
        << std::is_same_v<T0, T1> << "\n"

int main() {
    std::cout << std::boolalpha;

    TEST(int, int);
    TEST(int, unsigned);
    TEST(int, long);
    TEST(const int, int);
    TEST(int&, int);
    TEST(int&, int&);
    TEST(const char*, const char*);

    return 0;
}
```

```
int == int: true      
int == unsigned: false
int == long: false    
const int == int: false
int& == int: false
int& == int&: true
const char* == const char*: true
```
