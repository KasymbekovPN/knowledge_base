---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_trivial_v<T>` из заголовка `<type_traits>` проверяет, является ли тип **тривиальным (trivial)**.

> 🔧 Подключается через:
```cpp
#include <type_traits>
```

### Что значит "тривиальный" тип?

Тип `T` считается тривиальным (`trivial`), если **все его специальные функции** (конструкторы, деструктор, операторы копирования) являются **тривиальными**, то есть:

- Не пользовательские,
- Не удалённые,
- И не вызывают никакой сложной логики.

✅ Такие объекты можно:
- Копировать как блок байтов (`memcpy`),
- Размещать в статической памяти без инициализации,
- Передавать между C и C++ (если layout тоже стандартный).

### Условия для `std::is_trivial_v<T> == true`

Тип `T` должен удовлетворять всем условиям:
1. Иметь **тривиальный конструктор по умолчанию**.
2. Иметь **тривиальный деструктор**.
3. Иметь **тривиальный конструктор копирования**.
4. Иметь **тривиальный оператор присваивания**.
5. Не быть объединением с нетривиальными членами.

```cpp
#include <iostream>
#include <type_traits>
#include <cstring>

struct SimplePoint {
    double x, y;
};

struct Message {
    std::string data; // not a trivial !

    Message(): data{""} {}
    Message(const std::string& _data): data{_data} {}
    Message(const Message&) = default;
    Message& operator=(const Message&) = default;
};

template<typename T>
void test(T&, T&);

int main() {
    SimplePoint p0{1, 2}, p1;
    test(p0, p1);

    Message m0{"hello"}, m1;
    test(m0, m1);

    return 0;
}

template<typename T>
void test(T& _dst, T& _src) {
    if constexpr (std::is_trivial_v<T>) {
        std::memcpy(&_dst, &_src, sizeof(T));
        std::cout << "Copied with memcpy";
    } else {
        _dst = _src;
        std::cout << "Copied with assignment";
    }
    std::cout << std::endl;
}
```

```
Copied with memcpy
Copied with assignment
```
