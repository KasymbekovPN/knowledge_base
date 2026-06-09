---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_copy_constructible_v<T>` из заголовка `<type_traits>` проверяет, можно ли **скопировать объект типа `T`**, то есть создать его из другого объекта того же типа с помощью копирующего конструктора:

```cpp
T obj1;
T obj2 = obj1; // или T obj2(obj1);
```

### Что значит "copy constructible"?

Тип `T` является **copy constructible**, если:
- У него есть доступный копирующий конструктор,
- И он не удалён (`= delete`),
- И не приватный (если используется вне класса).

✅ Это включает:
- Примитивные типы: `int`, `double`
- Структуры и классы без `= delete` копирования
- Типы со сгенерированным компилятором `T(const T&)`

❌ Не включает:
- Классы с `T(const T&) = delete;`
- Классы с `private` копирующим конструктором
- Некоторые RAII-типы: например, `std::unique_ptr` — **не copyable**, но move-only

```cpp
#include <iostream>
#include <type_traits>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_copy_constructible_v<T> << "\n";

struct Point {
    int x, y;
};

class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
};

class Copyable {
public:
    Copyable() = default;
    Copyable(const Copyable&) = default;
};

using UniqInteger = std::unique_ptr<int>;

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(double);
    TEST(Point);
    TEST(NonCopyable);
    TEST(Copyable);
    TEST(UniqInteger);
    TEST(std::string);

    return 0;
}
```

```
int: true
double: true
Point: true
NonCopyable: false
Copyable: true
UniqInteger: false
std::string: true
```
