---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::contiguous_range` — самый сильный стандартный range concept в иерархии диапазонов C++20.

Он означает:
> элементы диапазона лежат в непрерывной области памяти, как в обычном массиве.

# Иерархия

```text
range
  ↓
input_range
  ↓
forward_range
  ↓
bidirectional_range
  ↓
random_access_range
  ↓
contiguous_range
```

Каждый `contiguous_range` автоматически является:
- `random_access_range`
- `bidirectional_range`
- `forward_range`
- `input_range`
- `range`

# Что проверяет concept

Упрощённо:

```cpp
template<class R>
concept contiguous_range =
    std::ranges::random_access_range<R> &&
    std::contiguous_iterator<
        std::ranges::iterator_t<R>
    >;
```

То есть:
1. диапазон поддерживает произвольный доступ;
2. его итератор является `contiguous_iterator`.

# Главная гарантия

Для диапазона существует указатель на первый элемент:

```cpp
std::ranges::data(r)
```

и элементы расположены подряд:

```text
elem0 elem1 elem2 elem3 ...
^^^^^
непрерывная память
```

# Таблица

|Тип|random_access_range|contiguous_range|
|---|---|---|
|`std::vector`|✅|✅|
|`std::array`|✅|✅|
|`std::span`|✅|✅|
|`int[N]`|✅|✅|
|`std::deque`|✅|❌|
|`std::list`|❌|❌|
|`std::forward_list`|❌|❌|

# Итог

`std::ranges::contiguous_range` гарантирует:
- диапазон поддерживает произвольный доступ;
- элементы находятся в непрерывной памяти;
- работает `std::ranges::data(r)`;
- итератор удовлетворяет `std::contiguous_iterator`;
- диапазон можно безопасно рассматривать как массив элементов;
- примеры: `std::vector`, `std::array`, `std::span`, `std::string`, массивы;
- не подходят: `std::deque`, `std::list`, `std::forward_list`.

```cpp
#include <iostream>
#include <vector>
#include <ranges>

struct NonRange {};

struct Range {
    static const size_t SIZE{5};
    int data[SIZE] {1, 2, 3, 4, 5};

    int* begin() {
        return data;
    }

    int* end() {
        return data + SIZE;
    }
};

template<std::ranges::random_access_range R>
void test(R&& _range) {
    std::cout << std::ranges::data(_range)[0] << std::endl;
}

int main() {
    test(std::vector<int>({11, 21, 31}));
    test(Range());
    // test(NonRange()); // Error

    return 0;
}
```

```
11
1
```
