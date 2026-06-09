---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_invocable_r_v<R, F, Args...>` из заголовка `<type_traits>` проверяет, **можно ли вызвать объект типа `F` с аргументами типов `Args...`** и будет ли его **возвращаемое значение неявно преобразуемо к типу `R`**.

### Что значит "invocable with return type `R`"?

`std::is_invocable_r_v<R, F, Args...>` возвращает `true`, если:
1. `F` можно вызвать от `Args...` (`std::is_invocable_v<F, Args...>`),
2. Результат этого вызова можно **неявно преобразовать** к типу `R`.

✅ Это мощный инструмент для метапрограммирования: вы можете выбирать перегрузки, реализации или проверять контракты на этапе компиляции.

```cpp
#include <iostream>
#include <type_traits>

#define TEST(R, T, ...) \
    std::cout \
        << #R " " #T "(" #__VA_ARGS__ "):" \
        << std::is_invocable_r_v<R, T, __VA_ARGS__> << "\n"

int func (double _input) {
    return static_cast<int>(_input * 2);
}

auto lambda = [](int _a, int _b) -> double {
    return _a + _b;
};

int main() {
    std::cout << std::boolalpha;

    TEST(int, decltype(func), double);
    TEST(double, decltype(func), double);
    TEST(void, decltype(func), double);

    TEST(double, decltype(lambda), int, int);
    TEST(float, decltype(lambda), int, int);
    TEST(int, decltype(lambda), int, int);

    TEST(std::string, decltype(lambda), int, int);

    return 0;
}
```


```
int decltype(func)(double):true
double decltype(func)(double):true
void decltype(func)(double):true
double decltype(lambda)(int, int):true
float decltype(lambda)(int, int):true
int decltype(lambda)(int, int):true
std::string decltype(lambda)(int, int):false
```
