---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::bidirectional_range` — concept из C++20 (`<ranges>`), который означает:

> диапазон можно обходить как вперёд, так и назад.

Он требует, чтобы iterator диапазона удовлетворял `std::bidirectional_iterator`.

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

# Что добавляет bidirectional_range

Для `forward_range` есть:

```cpp
++it;
```

Для `bidirectional_range` дополнительно гарантируется:

```cpp
--it;
```

# Что гарантируется

Для любого iterator диапазона:

```cpp
auto it = begin(r);

++it;
--it;
```

После этого:

```cpp
it
```

вернётся в исходное положение.

# Таблица

|Concept|`++it`|`--it`|
|---|---|---|
|`input_range`|✅|❌|
|`forward_range`|✅|❌|
|`bidirectional_range`|✅|✅|
|`random_access_range`|✅|✅|

# Отличие от random_access_range

`bidirectional_range` гарантирует только шаг назад:

```cpp
--it;
```

Но не гарантирует:

```cpp
it += 10;
it -= 10;
it[5];
it + 100;
```

Эти операции появляются только у `random_access_range`.

# Итог

`std::ranges::bidirectional_range` означает:
- диапазон поддерживает чтение и многократный проход;
- iterator удовлетворяет `std::bidirectional_iterator`;
- доступны операции `++it` и `--it`;
- можно обходить диапазон в обе стороны;
- примеры: `std::vector`, `std::list`, `std::deque`, `std::span`, массивы;
- не подходит для `std::forward_list`, поскольку его итератор не умеет двигаться назад.

```cpp
#include <iostream>
#include <vector>
#include <ranges>

struct NonRange {};

struct Range {
    int data[5] {1, 2, 3, 4, 5};

    int* begin() {
        return data;
    }

    int* end() {
        return data + 5;
    }
};

template<std::ranges::bidirectional_range R>
void test(R&& _range) {
    for (auto& item: _range){
        std::cout << item << " ";
    }
    std::cout << std::endl;

    auto&& it = std::ranges::end(_range);
    while (it != std::ranges::begin(_range)) {
        --it;
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

int main() {
    test(std::vector<int>({1, 2, 3}));
    test(Range());
    // test(NonRange()); // Error

    return 0;
}
```

```
1 2 3 
3 2 1 
1 2 3 4 5 
5 4 3 2 1 
```
