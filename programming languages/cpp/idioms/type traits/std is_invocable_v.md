---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_invocable_v<F, Args...>` из заголовка `<type_traits>` проверяет, **можно ли вызвать объект типа `F` с аргументами типов `Args...`**.

### Что значит "invocable"?

Тип `F` считается **invocable** для `Args...`, если выражение:
```cpp
std::invoke(std::declval<F>(), std::declval<Args>()...)
```
корректно компилируется.

✅ Это включает:
- Функции,
- Лямбды,
- Указатели на функции/методы,
- Объекты с `operator()`,
- Любые вызываемые (callable) типы.

```cpp
#include <iostream>
#include <type_traits>

#define TEST(T, ...) \
    std::cout \
        << #T "<" #__VA_ARGS__ ">:" \
        << std::is_invocable_v<T, __VA_ARGS__> << "\n"

void func(int _value) {}

auto lambda = [](double _value) { return 2 * _value; };

struct Callable {
    int operator()(int _a, int _b) { return _a + _b; }
};

int main() {
    std::cout << std::boolalpha;

    TEST(decltype(func), int);
    TEST(decltype(lambda), int);
    TEST(decltype(lambda), double);
    TEST(Callable, int, int);

    return 0;
}
```

```
decltype(func)<int>:true
decltype(lambda)<int>:true
decltype(lambda)<double>:true
Callable<int, int>:true
```
