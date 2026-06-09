---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::derived_from<Derived, Base>`** из заголовка `<concepts>` проверяет, что тип `Derived` является **базовым или производным** от `Base`, с учётом правил наследования в C++.

### Что делает `std::derived_from<Derived, Base>`?

Он возвращает `true`, если:
1. `Derived` и `Base` — классы,
2. `Derived` напрямую или косвенно унаследован от `Base`,
3. Или `Derived` и `Base` — один и тот же тип.

✅ Это безопасная и читаемая замена для ручной проверки через `std::is_base_of`.

```cpp
#include <iostream>
#include <concepts>

struct Animal {
    virtual ~Animal() = default;
};

struct Dog: Animal {};

struct Cat: Animal {};

struct Bulldog: Dog {};

struct Human {};

template<std::derived_from<Animal> T>
void make_sound(const T&);

template<typename T>
requires std::derived_from<T, Animal>
void process(T&);

int main() {
    Animal animal;
    Cat cat;
    Dog dog;
    Bulldog bulldog;
    Human human;

    make_sound(animal);
    make_sound(cat);
    make_sound(dog);
    make_sound(bulldog);
    // make_sound(human); // Error

    process(animal);
    process(cat);
    process(dog);
    process(bulldog);
    // process(human); // Error

    return 0;
}

template<std::derived_from<Animal> T>
void make_sound(const T& _input) {
    std::cout << "Some animal sound" << std::endl;
}

template<typename T>
requires std::derived_from<T, Animal>
void process(T& _input) {
    std::cout << "Processing an aninal" << std::endl;
}
```


```
Some animal sound
Some animal sound
Some animal sound
Some animal sound
Processing an aninal
Processing an aninal
Processing an aninal
Processing an aninal
```
