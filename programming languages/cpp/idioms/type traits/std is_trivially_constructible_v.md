---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_trivially_constructible_v<T, Args...>` проверяет, **можно ли создать объект типа `T` из аргументов типа `Args...` тривиальным способом** — то есть без вызова кода (например, просто копированием байтов).
### Что значит "тривиально конструируемый"?

Тип `T` является **тривиально конструируемым** от `Args...`, если:
- Конструктор не определён пользователем,
- Не требуется выполнение кода при инициализации,
- Можно использовать `memcpy` или нулевую инициализацию.

✅ Это позволяет компилятору оптимизировать создание объектов.

```cpp
#include <iostream>
#include <type_traits>

#define TEST(T, ...) \
    std::cout \
        << #T "<" #__VA_ARGS__ ">: " \
        << std::is_trivially_constructible_v<T, __VA_ARGS__> << "\n"

struct Point {
    int x, y;
};

struct TrivialClass {
    int a;
    double b;
};

class NotTrivialClass {
public:
    int value {0};
    NotTrivialClass() {}
};

int main() {
    std::cout << std::boolalpha;

    TEST(int, int);

    TEST(Point, Point);
    TEST(Point, int, int);
    TEST(Point, int, Point);

    TEST(TrivialClass, int, double);
    TEST(TrivialClass, int, std::string);

    TEST(NotTrivialClass, int);

    return 0;
}
```

```
int<int>: true
Point<Point>: true
Point<int, int>: true
Point<int, Point>: false
TrivialClass<int, double>: true
TrivialClass<int, std::string>: false
NotTrivialClass<int>: false
```

---

## ✅ Пример 1: Базовая проверка

```cpp
#include <iostream>
#include <type_traits>

#define PRINT(T, ...) \
    std::cout << #T "<" #__VA_ARGS__ ">: " \
              << std::is_trivially_constructible_v<T, __VA_ARGS__> << "\n"

struct Point {
    int x, y;
};

struct TrivialClass {
    int a;
    double b;
};

class NonTrivial {
public:
    NonTrivial() { /* что-то делаем */ }
    int value = 0;
};

int main() {
    std::cout << std::boolalpha;

    // Тривиальные типы
    PRINT(int, );                     // true — тривиальный конструктор по умолчанию
    PRINT(Point, );                   // true — POD-структура
    PRINT(TrivialClass, );           // true — нет пользовательских конструкторов

    // Но можно и с параметрами?
    PRINT(double, double);            // false! Несмотря на простоту, нет тривиального copy/move?
    // На самом деле: тривиальность обычно только для default/move/copy

    // С указателями
    PRINT(int*, );                    // true
    PRINT(int*, int*);               // true — можно тривиально скопировать указатель

    // НЕ тривиально
    PRINT(NonTrivial, );             // false — пользовательский конструктор
}
```

### Вывод:
```
int<>: true
Point<>: true
TrivialClass<>: true
double<double>: false
int*<>: true
int*<int*>: true
NonTrivial<>: false
```

> ⚠️ Обратите внимание:  
> `std::is_trivially_constructible_v<T>` почти всегда `true` ↔ если `T` имеет **неопределённый (trivial) конструктор по умолчанию**.
