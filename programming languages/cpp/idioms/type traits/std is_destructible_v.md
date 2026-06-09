---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_destructible_v<T>` из заголовка `<type_traits>` проверяет, **можно ли безопасно уничтожить объект типа `T`**.

### Что значит "destructible"?

Тип `T` является **destructible**, если выражение:
```cpp
t.~T()
```
или просто выход из области видимости объекта `T t;` **не вызывает неопределённого поведения**.

✅ Это включает:
- Все типы с доступным (не `private`, не `= delete`) деструктором,
- Фундаментальные типы (`int`, `double`),
- Массивы,
- Классы со сгенерированными или пользовательскими деструкторами.

❌ Не включает:
- Типы с `~T() = delete`,
- Типы с `private` деструктором (если используется вне класса).

```cpp
#include <iostream>
#include <type_traits>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_destructible_v<T> << "\n"

struct Point {
    int x, y;
};

class NonDestructible {
public:
    ~NonDestructible() = delete;
};

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(double);
    TEST(Point);
    TEST(NonDestructible);
    TEST(std::string);
    TEST(std::unique_ptr<int>);

    return 0;
}
```

```
int: true
double: true
Point: true
NonDestructible: false
std::string: true
std::unique_ptr<int>: true
```
