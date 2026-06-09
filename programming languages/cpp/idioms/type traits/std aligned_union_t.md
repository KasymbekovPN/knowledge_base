---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::aligned_union_t<Len, Types...>`** из заголовка `<type_traits>` определяет тип, подходящий для хранения **любого из перечисленных типов**, с учётом выравнивания.

### Что делает `std::aligned_union_t<Len, ...>`?

Он возвращает:
1. **Размер**: как минимум `Len` **или** размер самого большого типа из списка,
2. **Выравнивание**: на границу самого строгего (наибольшего) `alignof(T)` среди всех `Types...`.

✅ Используется при реализации:
- Собственных `union`-подобных контейнеров,
- `variant` (до C++17),
- Объектных пулов,
- Агрегаторов разных типов.

## ⚠️ Важно: Устарело в C++23

Начиная с **C++23**, `std::aligned_union_t` и `std::aligned_union` объявлены **устаревшими (deprecated)**.

👉 Современная альтернатива:
```cpp
alignas(Align) std::byte storage[Size];
```
или использование стандартного `std::variant`.

```cpp
#define _ENABLE_EXTENDED_ALIGNED_STORAGE
// #define _DISABLE_EXTENDED_ALIGNED_STORAGE

#include <iostream>
#include <type_traits>
#include <new>

struct Small {
    int x;
};

struct Big {
    double data[10];
};

struct Aligned {
    alignas(16) char buffer[48];
};

using Storage = std::aligned_union_t<2, Small, Big, Aligned>;

int main() {
    alignas(Storage) char raw_memory[sizeof(Storage)];
    void* mem = raw_memory;

    Big* b = new(mem) Big{};
    b->data[0] = 3.14;

    std::cout << "Value: " << b->data[0] << std::endl;

    b->~Big();

    return 0;
}
```

```
Value: 3.14
```
