---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::same_as<T, U>`** из заголовка `<concepts>` проверяет, являются ли два типа **точно одинаковыми**.

### Что делает `std::same_as<T, U>`?

Он возвращает `true`, если:
- `T` и `U` — **один и тот же тип**, с учётом `const`, `volatile`, ссылок.
- И при этом: `std::convertible_to<T, U>` и `std::convertible_to<U, T>` → обеспечивает симметрию.

✅ Это **более строгое сравнение**, чем `std::is_same_v<T, U>`, но часто используется в тех же целях.

```cpp
#include <iostream>
#include <concepts>

template<typename T, typename U>
void test();

template<typename T>
requires std::same_as<T, int>
void print(T);

template<typename T>
requires std::same_as<T, double>
void print(T);

int main() {
    test<int, int>();
    test<int, const int>();
    test<int, int&>();
    test<int*, int*>();
    test<int, double>();

    print(42);
    print(12.34);
    // print(333.0f); // Error

    return 0;
}

template<typename T, typename U>
void test() {
    if constexpr (std::same_as<T, U>) {
        std::cout << "Same";
    } else {
        std::cout << "Different";
    }
    std::cout << std::endl;
}

template<typename T>
requires std::same_as<T, int>
void print(T _input) {
    std::cout << "int: " << _input << std::endl;
}

template<typename T>
requires std::same_as<T, double>
void print(T _input) {
    std::cout << "double: " << _input << std::endl;
}
```

```
Same
Different
Different
Same
Different
int: 42
double: 12.34
```
