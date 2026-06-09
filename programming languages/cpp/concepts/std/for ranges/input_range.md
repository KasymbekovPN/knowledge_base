---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::input_range` — один из базовых concepts библиотеки ranges (C++20).

Он проверяет:
> является ли тип диапазоном, элементы которого можно читать хотя бы один раз.

# Определение

Упрощённо:

```cpp
template<class R>
concept input_range =
    std::ranges::range<R> &&
    std::input_iterator<
        std::ranges::iterator_t<R>
    >;
```

То есть:
1. у объекта есть `begin()` и `end()`;
2. его iterator удовлетворяет `std::input_iterator`.

# Что можно делать с input_range

Для любого `input_range` гарантированно работают:

```cpp
auto it = std::ranges::begin(r);

*it;
++it;
```

# Главное ограничение

`input_range` не гарантирует многократный проход.

Например поток ввода:

```cpp
#include <sstream>
#include <iterator>
```

может предоставлять только одноразовый обход.

Поэтому:

```text
input_range
```

означает:

```text
можно читать элементы,
но не обязательно несколько раз
```

# Сравнение с другими range concepts

|Concept|Можно читать|Многократный проход|
|---|---|---|
|`input_range`|✅|❌|
|`forward_range`|✅|✅|
|`bidirectional_range`|✅|✅|
|`random_access_range`|✅|✅|

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

Каждый следующий concept сильнее предыдущего.

# Проверка разных контейнеров

```cpp
#include <ranges>
#include <vector>
#include <list>
#include <forward_list>

static_assert(std::ranges::input_range<std::vector<int>>);
static_assert(std::ranges::input_range<std::list<int>>);
static_assert(std::ranges::input_range<std::forward_list<int>>);
```

Все они являются как минимум `input_range`.

# Итог

`std::ranges::input_range` гарантирует, что:
- объект является диапазоном (`range`);
- его элементы можно читать через iterator;
- iterator поддерживает `*it` и `++it`;
- диапазон можно обходить последовательно;
- многократный проход **не гарантируется** (для этого нужен `std::ranges::forward_range`).

```cpp
#include <iostream>
#include <ranges>
#include <vector>

struct NonRange {};

struct Range {
    int data[3]{1, 2, 3};

    int* begin() {
        return data;
    }

    int *end() {
        return data + 3;
    }
};

template<std::ranges::input_range R>
void test(R&& _range) {
    for (auto&& item: _range) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    test(std::vector<int>({11, 12, 13}));
    test(Range());
    // test(NonRange()); // Error

    return 0;
}
```

```
11 12 13 
1 2 3
```
