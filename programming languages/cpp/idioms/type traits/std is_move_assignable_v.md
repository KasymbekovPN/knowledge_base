---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_move_assignable_v<T>` из заголовка `<type_traits>` проверяет, можно ли **присвоить объекту типа `T` значение временного объекта (rvalue)**, то есть использовать **оператор перемещающего присваивания**:

```cpp
T a, b;
a = std::move(b); // ← работает, если T — move-assignable
```

### Что значит "move assignable"?

Тип `T` является **move assignable**, если выражение:
```cpp
std::declval<T>() = std::declval<T&&>()
```
допустимо — то есть существует и доступен оператор:
```cpp
T& operator=(T&&);
```

✅ Это позволяет эффективно **перемещать ресурсы** вместо копирования.

```cpp
#include <iostream>
#include <type_traits>
#include <vector>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_move_assignable_v<T> << "\n"

struct Point {
    int x, y;
};

class NonMoveAssignable {
public:
    NonMoveAssignable() = default;
    NonMoveAssignable& operator=(NonMoveAssignable&&) = delete;
};

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(double);
    TEST(Point);
    TEST(std::string);
    TEST(std::vector<int>);
    TEST(std::unique_ptr<int>);
    TEST(NonMoveAssignable);

    return 0;
}
```

```
int: true        
double: true     
Point: true      
std::string: true
std::vector<int>: true
std::unique_ptr<int>: true
NonMoveAssignable: false
```
