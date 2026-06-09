---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_scalar_v<T>` из заголовка `<type_traits>` проверяет, является ли тип **скалярным (scalar type)**.

> 🔧 Подключается через:
```cpp
#include <type_traits>
```

---

### Что такое скалярный тип?

`std::is_scalar_v<T>` возвращает `true`, если `T` — это:

✅ Целые числа:  
- `int`, `char`, `bool`, `unsigned long` и т.д.

✅ Числа с плавающей точкой:  
- `float`, `double`, `long double`

✅ Указатели:  
- `int*`, `void*`, `const char*`, `MyClass*`

✅ Указатели на члены класса:  
- `int MyClass::*`
- `void (MyClass::*)()`

✅ Перечисления (`enum`, `enum class`)  
✅ `std::nullptr_t`

> 💡 Скалярные типы — это **все "простые" типы**, которые можно копировать как блок байтов.

### Не являются скалярными:
- Классы, структуры
- Массивы
- Объединения (`union`)
- Функции
- Ссылки (`int&`)
- `void` (сам по себе)

```cpp
#include <iostream>
#include <type_traits>

enum Color { Red };

struct Widget {};

template<typename T>
void test();

int main() {
    test<int>();
    test<double>();
    test<bool>();
    test<char*>();
    test<const int*>();
    test<void*>();
    test<std::nullptr_t>();
    test<Color>();
    test<Widget>();
    test<int[4]>();
    test<void>();
    test<int&>();
    test<void()>();

    return 0;
}

template<typename T>
void test() {
    if constexpr (std::is_scalar_v<T>) {
        std::cout << "A scalar object";
    } else {
        std::cout << "Not a scalar object";
    }
    std::cout << std::endl;
}
```

```
A scalar object
A scalar object
A scalar object
A scalar object
A scalar object
A scalar object
A scalar object
A scalar object
Not a scalar object
Not a scalar object
Not a scalar object
Not a scalar object
Not a scalar object
```
