---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_volatile_v<T>` из заголовка `<type_traits>` проверяет, является ли **сам тип `T`** квалифицированным как `volatile`.

> 🔧 Подключается через:
```cpp
#include <type_traits>
```

### Что такое `volatile`?

Ключевое слово `volatile` указывает компилятору, что значение переменной может изменяться **внешними факторами** (например, аппаратными регистрами, сигналами), и поэтому **не должно оптимизироваться**.

```cpp
volatile int sensor_value; // Может меняться вне программы
```

`std::is_volatile_v<T>` возвращает `true`, если `T` — это `volatile` тип, например:

- `volatile int`
- `int volatile* const` (const указатель на volatile данные)
- `volatile char[5]`

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void test();

int main() {
    test<int>();
    test<const int>();
    test<const volatile int>();
    test<int&>();
    test<const int&>();
    test<const volatile int&>();

    return 0;
}

template<typename T>
void test() {
    if constexpr (std::is_volatile_v<T>) {
        std::cout << "A volitile";
    } else if (
        std::is_reference_v<T> &&
        std::is_volatile_v<std::remove_reference_t<T>>
    ) {
        std::cout << "A volitile refernce";
    } else {
        std::cout << "Not a volitile";
    }
    std::cout << std::endl;
}
```

```
Not a volitile
Not a volitile
A volitile
Not a volitile
Not a volitile
A volitile refernce
```
