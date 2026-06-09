---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_nothrow_constructible_v<T, Args...>` проверяет, **можно ли создать объект типа `T` из аргументов типа `Args...` без выброса исключений**.
### Что значит "nothrow constructible"?

Тип `T` является **nothrow-конструируемым** от `Args...`, если выражение:
```cpp
T(std::declval<Args>()...)
```
гарантированно **не бросает исключений** (объявлено как `noexcept`).

✅ Это важно для:
- Исключения-безопасного кода,
- Оптимизации в STL (`vector::push_back`, `swap`),
- Гарантии безопасности при перемещении.

```cpp
#include <iostream>
#include <type_traits>

#define TEST(T, ...) \
    std::cout \
        << #T "<" #__VA_ARGS__ ">:" \
        << std::is_nothrow_constructible_v<T, __VA_ARGS__> << "\n";

struct Trivial {
    int value;
};

struct ThrowClass {
    ThrowClass() {}
};

struct NoThrowClass {
    NoThrowClass() noexcept {}
};

class Wrapper {
    int value;
public:
    explicit Wrapper(int _value) noexcept:
        value{_value} {}
};

int main() {
    std::cout << std::boolalpha;

    TEST(Trivial);
    TEST(Trivial, int);
    TEST(NoThrowClass);
    TEST(ThrowClass);
    TEST(Wrapper, int);
    TEST(int);

    return 0;
}
```

```
Trivial<>:true
Trivial<int>:true
NoThrowClass<>:true
ThrowClass<>:false
Wrapper<int>:true
int<>:true
```
