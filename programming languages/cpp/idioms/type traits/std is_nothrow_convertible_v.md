---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_nothrow_convertible_v<From, To>` (доступна с **C++20**) проверяет, можно ли **неявно преобразовать** объект типа `From` в тип `To` **без выброса исключений**.

### Что значит "nothrow convertible"?

Выражение:
```cpp
To obj = std::declval<From>();
```
должно быть:
1. Валидным (т.е. `std::is_convertible_v<From, To>` → `true`),
2. Гарантированно не бросать исключения (объявлено как `noexcept`).

✅ Это полезно для:
- Исключения-безопасного кода,
- Оптимизаций в STL (`vector::resize`, `swap`),
- Проверки контрактов на этапе компиляции.

```cpp
#include <iostream>
#include <type_traits>

#define TEST(F, T) \
    std::cout \
        << #F " -> " #T " : " \
        << std::is_nothrow_convertible_v<F, T> << "\n"

struct Base {};

struct Derived: Base {};

int main() {
    std::cout << std::boolalpha;

    TEST(int, double);
    TEST(long, int);
    TEST(float, long double);

    TEST(Derived*, Base*);
    TEST(nullptr_t, void*);

    TEST(Base*, Derived*);

    return 0;
}
```

```
int -> double : true
long -> int : true
float -> long double : true
Derived* -> Base* : true
nullptr_t -> void* : true
Base* -> Derived* : false
```
