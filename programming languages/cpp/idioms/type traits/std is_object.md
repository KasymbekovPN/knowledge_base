---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_object_v<T>` из заголовка `<type_traits>` проверяет, является ли тип **объектом** — то есть может быть представлен в виде блока памяти и использован для создания переменных.

> 🔧 Подключается через:
```cpp
#include <type_traits>
```

### Что такое "object type"?

`std::is_object_v<T>` возвращает `true`, если `T` — это **тип объекта**, то есть:

✅ Да — можно создать переменную:
- Все классы, структуры
- Встроенные типы: `int`, `double`, `bool`, `char`
- Указатели: `int*`, `void*`
- Массивы: `int[5]`, `char[]`
- Объединения (`union`)
- Перечисления (`enum`)

❌ Нет — нельзя создать переменную:
- `void`
- Функции (например, `void(int)`)
- Ссылки (`int&`, `const T&`)

```cpp
#include <iostream>
#include <type_traits>

struct Value { int x; };

enum Color { Red };

union U { int i; float f; };

template<typename T>
void test();

int main() {
    test<int>();
    test<bool>();
    test<std::string>();
    test<int*>();
    test<Value>();
    test<Color>();
    test<U>();

    test<void>();
    test<int&>();

    return 0;
}

template<typename T>
void test() {
    if constexpr (std::is_object_v<T>) {
        std::cout << "A object type";
    } else {
        std::cout << "Not a object type";
    }
    std::cout << std::endl;
}
```

```
A object type
A object type
A object type
A object type
A object type
A object type
A object type
Not a object type
Not a object type
```

