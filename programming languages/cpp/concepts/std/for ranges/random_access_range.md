---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::random_access_range` — concept из C++20 (`<ranges>`), который означает:

> диапазон поддерживает произвольный доступ к элементам за константное время.

Итератор такого диапазона должен удовлетворять `std::random_access_iterator`.

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

# Что добавляет random_access_range

Для `bidirectional_range` есть:

```cpp
++it;
--it;
```

Для `random_access_range` дополнительно доступны:

```cpp
it += n;
it -= n;

it + n;
it - n;

it[n];

it1 - it2;
```

# Сравнение

|Тип|random_access_range|contiguous_range|
|---|---|---|
|`std::vector`|✅|✅|
|`std::array`|✅|✅|
|`std::span`|✅|✅|
|`int[N]`|✅|✅|
|`std::deque`|✅|❌|
|`std::list`|❌|❌|

# Итог

`std::ranges::random_access_range` означает:
- диапазон поддерживает произвольный доступ;
- iterator удовлетворяет `std::random_access_iterator`;
- доступны `it + n`, `it - n`, `it[n]`;
- расстояние между итераторами вычисляется за O(1);
- примеры: `std::vector`, `std::array`, `std::span`, `std::deque`, массивы;
- не подходит для `std::list` и `std::forward_list`.

```cpp
#include <iostream>
#include <ranges>
#include <vector>

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
    std::cout << std::ranges::begin(_range)[2] << std::endl;
}

int main() {
    test(std::vector<int>({11, 12, 13}));
    test(Range());
    // test(NonRange()); // Error

    return 0;
}
```

```
13
3
```
