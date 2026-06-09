---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::range` — базовый range concept из C++20 из `<ranges>`.

Он проверяет:

> можно ли получить `begin()` и `end()`.

# Идея

Range — это объект, который можно перебирать:

```cpp
begin(obj)
end(obj)
```

# Что проверяет concept

Упрощенно:

```cpp
std::ranges::begin(r)
std::ranges::end(r)
```

# Важный момент

`range` НЕ требует:
- random access
- size
- contiguous memory

Только возможность итерироваться.

# Реальные range типы

| Тип           | range |
| ------------- | ----- |
| `std::vector` | yes   |
| `std::list`   | yes   |
| `std::array`  | yes   |
| `std::string` | yes   |

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

```cpp
#include <iostream>
#include <vector>
#include <concepts>

struct NonRange {};

struct Range {
    int data[3] = {1, 2, 3};

    int* begin() {
        return data;
    }

    int* end() {
        return data + 3;
    }
};

template<std::ranges::range R>
void test(R&& _input) {
    for (auto&& item: _input) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<int> v {42, 43, 44};
    test(v);

    test(Range());
    // test(NonRange()); // Error

    return 0;
}
```

```
42 43 44 
1 2 3
```
