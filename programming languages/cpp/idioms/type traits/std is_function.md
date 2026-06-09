---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_function_v<T>` из заголовка `<type_traits>` проверяет, является ли тип **функцией** (не указателем на функцию, а именно *функциональным типом*).

> 🔧 Подключается через:
```cpp
#include <type_traits>
```

### Что такое "функциональный тип"?

В C++ можно объявлять типы как функции:

```cpp
void func(int); // функция
using FuncType = void(int); // тип функции
```

⚠️ Обратите внимание:
- `void(int)` — это **тип функции**
- `void(*)(int)` — это **указатель на функцию**

`std::is_function_v<T>` возвращает `true` только для первых.

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void test(const T&);

void func();

int main() {
    void(*pf)() = &func;

    test(42);
    test("Hello");
    test(func);
    test(pf);

    return 0;
}

template<typename T>
void test(const T& _input) {
    if constexpr (std::is_function_v<T>) {
        std::cout << "This is a function type";
    } else if (std::is_pointer_v<T> &&
               std::is_function_v<std::remove_pointer_t<T>>) {
        std::cout << "This is a pointer to function";
    } else {
        std::cout << "Other";
    }
    std::cout << std::endl;
}

void func() {}
```

```
Other
Other
This is a function type
This is a pointer to function
```
