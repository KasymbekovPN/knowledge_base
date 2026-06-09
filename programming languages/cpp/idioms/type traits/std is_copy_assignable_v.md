---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_copy_assignable_v<T>` из заголовка `<type_traits>` проверяет, можно ли **присвоить один объект типа `T` другому**, используя **оператор копирующего присваивания**:

```cpp
T a, b;
a = b; // ← работает, если T является copy-assignable
```

### Что значит "copy assignable"?

Тип `T` является **copy assignable**, если выражение:
```cpp
std::declval<T>() = std::declval<const T&>()
```
допустимо и оператор `operator=(const T&)` доступен (не `= delete`, не `private`).

✅ Это включает:
- Фундаментальные типы (`int`, `double`)
- Структуры и классы с автоматически или явно определённым оператором присваивания
- Контейнеры: `std::string`, `std::vector`, и т.д.

❌ Не включает:
- Типы с `operator=(const T&) = delete`
- Некоторые RAII-типы: например, `std::unique_ptr` — **не copy-assignable**
- Массивы (в C++ массивы нельзя присваивать напрямую)

```cpp
#include <iostream>
#include <vector>
#include <type_traits>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_copy_assignable_v<T> << "\n";

struct Point {
    int x, y;
};

class NonCopyAssignable {
public:
    NonCopyAssignable() = default;
    NonCopyAssignable& operator=(const NonCopyAssignable&) = delete;
};

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(double);
    TEST(Point);
    TEST(std::string);
    TEST(std::vector<int>);
    TEST(NonCopyAssignable);
    TEST(std::unique_ptr<float>);

    return 0;
}
```

```
int: true
double: true
Point: true
std::string: true
std::vector<int>: true
NonCopyAssignable: false
std::unique_ptr<float>: false
```
