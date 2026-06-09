---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_move_constructible_v<T>` из заголовка `<type_traits>` проверяет, можно ли **создать объект типа `T` из rvalue-ссылки** (`T&&`), то есть использовать **конструктор перемещения**.
### Что значит "move constructible"?

Тип `T` является **move constructible**, если существует конструктор:
```cpp
T(T&& other);
```
и он доступен (не `= delete`, не `private`).

✅ Это позволяет эффективно "перемещать" ресурсы вместо копирования.

```cpp
#include <iostream>
#include <type_traits>
#include <thread>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_move_constructible_v<T> << "\n";

struct Point {
    int x, y;
};

class NonMovable {
    NonMovable() = default;
    NonMovable(NonMovable&&) = delete;
};

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(Point);
    TEST(std::string);
    TEST(std::unique_ptr<int>);
    TEST(std::thread);
    TEST(NonMovable);

    return 0;
}
```

```
int: true
Point: true
std::string: true
std::unique_ptr<int>: true
std::thread: true
NonMovable: false
```
