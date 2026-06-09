---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::forward_range` — concept из C++20 (`<ranges>`), который означает:

> диапазон можно читать **многократно**, а его iterator является `std::forward_iterator`.

Это более сильное требование, чем `std::ranges::input_range`.

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

# Что добавляет forward_range

Для `input_range` гарантировано:

```cpp
++it;
*it;
```

Но не гарантировано, что можно пройти диапазон второй раз.

Для `forward_range` гарантируется:
- многократный проход;
- копирование iterator-ов;
- два iterator-а могут независимо указывать на один элемент.

# Таблица

|Concept|Чтение|Повторный проход|
|---|---|---|
|`input_range`|✅|❌|
|`forward_range`|✅|✅|
|`bidirectional_range`|✅|✅|
|`random_access_range`|✅|✅|

# Практическое применение

Многие алгоритмы требуют именно:

```cpp
template<std::ranges::forward_range R>
```

если они:
- делают несколько проходов;
- сохраняют iterator-ы;
- сравнивают iterator-ы между собой.

# Итог

`std::ranges::forward_range` означает:
- это `input_range`;
- его iterator является `std::forward_iterator`;
- диапазон можно проходить многократно;
- iterator можно копировать;
- несколько iterator-ов могут одновременно ссылаться на один диапазон;
- примеры: `std::vector`, `std::list`, `std::forward_list`, `std::span`, массивы.

```cpp
#include <iostream>
#include <ranges>
#include <vector>

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

template<std::ranges::forward_range R>
void test(R&& _range) {
    for (auto& item: _range){
        std::cout << item << " ";
    }
    std::cout << std::endl;

    for (auto& item: _range){
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    test(std::vector({1, 2, 3}));
    test(Range());
    // test(NonRange()); // Error

    return 0;
}
```

```
1 2 3 
1 2 3 
1 2 3 4 5 
1 2 3 4 5
```
