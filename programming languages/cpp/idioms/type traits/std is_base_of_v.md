---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_base_of_v<Base, Derived>` из заголовка `<type_traits>` проверяет, является ли тип `Base` **базовым классом** для типа `Derived` (прямо или косвенно).

### Что делает `std::is_base_of_v<Base, Derived>`?

Возвращает `true`, если:
- `Base` — базовый класс `Derived`,
- Или `Base` и `Derived` — один и тот же класс,
- И оба являются **полными классами** (не forward declarations).

> ❗ Работает только с классами/структурами.  
> Для встроенных типов (`int`, `double`) — всегда `false`.

```cpp
#include <iostream>
#include <type_traits>

#define TEST(B, D)\
    std::cout \
        << #B " is base of " #D ": " \
        << std::is_base_of_v<B, D> << "\n"

struct Animal {
    virtual ~Animal() = default;
};

struct Dog: Animal {};

struct Cat: Animal {};

struct Bulldog: public Dog {};

int main() {
    std::cout << std::boolalpha;

    TEST(Animal, Dog);
    TEST(Animal, Bulldog);
    TEST(Dog, Bulldog);
    TEST(Cat, Dog);
    TEST(Dog, Dog);

    return 0;
}
```

```
Animal is base of Dog: true
Animal is base of Bulldog: true
Dog is base of Bulldog: true
Cat is base of Dog: false
Dog is base of Dog: true
```
