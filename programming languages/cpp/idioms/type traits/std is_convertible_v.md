---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_convertible_v<From, To>` из заголовка `<type_traits>` проверяет, можно ли **неявно преобразовать** объект типа `From` в тип `To`.

### Что значит "convertible"?

Выражение:
```cpp
To obj = std::declval<From>();
```
должно быть корректным (без явного приведения).

✅ Это включает:
- Приведение арифметических типов (`int → double`),
- Приведение указателей по иерархии наследования,
- Автоматические конверсии с помощью `operator T()` или конструкторов.

❌ Не включает:
- Явные приведения: `(To)from`, `static_cast`, `dynamic_cast`,
- Несовместимые типы.

```cpp
#include <iostream>
#include <type_traits>

#define TEST(F, T) \
    std::cout \
        << #F " -> " #T ": " \
        << std::is_convertible_v<F, T> << "\n"

struct Animal {
    virtual ~Animal() = default;
};

struct Dog: Animal {};

int main() {
    std::cout << std::boolalpha;

    TEST(int, double);
    TEST(double, int);
    TEST(float, long double);
    TEST(Dog*, Animal*);
    TEST(Animal*, Dog*);
    TEST(void*, int*);
    TEST(int*, void*);
    TEST(bool, int);
    TEST(nullptr_t, void*);

    return 0;
}
```

```
int -> double: true
double -> int: true
float -> long double: true
Dog* -> Animal*: true
Animal* -> Dog*: false
void* -> int*: false
int* -> void*: true
bool -> int: true
nullptr_t -> void*: true
```
