---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::output_range` — concept из C++20 (`<ranges>`), который проверяет:
> можно ли записывать значения определённого типа в элементы диапазона.

# Определение

Упрощённо:

```cpp
template<class R, class T>
concept output_range =
    std::ranges::range<R> &&
    std::output_iterator<
        std::ranges::iterator_t<R>,
        T
    >;
```

То есть:
1. `R` должен быть диапазоном (`range`);
2. его iterator должен быть `output_iterator` для типа `T`.

# Связь с output_iterator

Concept фактически проверяет:

```cpp
std::output_iterator<
    std::ranges::iterator_t<R>,
    T
>
```

Например:

```cpp
std::vector<int>::iterator
```

является:

```cpp
std::output_iterator<int>
```

поэтому весь контейнер становится:

```cpp
std::ranges::output_range<
    std::vector<int>,
    int
>
```

# Сравнение с input_range

### input_range

Можно читать:

```cpp
*it
```

### output_range

Можно писать:

```cpp
*it = value
```

# Таблица

|Concept|Чтение|Запись|
|---|---|---|
|`input_range`|✅|не обязательно|
|`output_range`|не обязательно|✅|
|`forward_range`|✅|зависит от iterator|
|`contiguous_range`|✅|зависит от iterator|

# Итог

`std::ranges::output_range<R, T>` означает:
- `R` является диапазоном (`range`);
- его элементы можно изменять через iterator;
- выражение

```cpp
*it = T(...)
```

корректно;
- concept используется для алгоритмов, которые записывают данные в диапазон.

```cpp
#include <iostream>
#include <ranges>
#include <vector>

template<std::ranges::output_range<int> R>
void test(R& _input) {
    for (auto it{_input.begin()}; it != _input.end(); ++it) {
        *it = 42;
    }
}

int main() {
    std::vector<int> vec(5);
    test(vec);
    for (auto&& item: vec) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

```
42 42 42 42 42
```
