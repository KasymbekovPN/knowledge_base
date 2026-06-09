---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_assignable_v<T, U>` из заголовка `<type_traits>` проверяет, можно ли присвоить значение типа `U` переменной типа `T`.

### Что значит "assignable"?

Выражение:
```cpp
std::declval<T>() = std::declval<U>();
```
должно быть корректным.

✅ Это **не то же самое**, что `std::is_convertible_v`, потому что:
- Проверяется именно **оператор присваивания**,
- Учитывается `const`, `volatile`, ссылки и специальные поведения классов.

```cpp
#include <iostream>
#include <type_traits>

#define TEST(T, U) \
    std::cout \
        << #T " = " #U ": " \
        << std::is_assignable_v<T, U> << "\n"

int main() {
    std::cout << std::boolalpha;

    TEST(int, int);
    TEST(int, double);
    TEST(double, int);

    TEST(int&, int);
    TEST(int&, const int&);
    TEST(const int&, int);

    TEST(int*, int*);
    TEST(void*, int*);
    TEST(int*, void*);

    return 0;
}
```

```
int = int: false
int = double: false
double = int: false
int& = int: true
int& = const int&: true
const int& = int: false
int* = int*: false
void* = int*: false
int* = void*: false
```
