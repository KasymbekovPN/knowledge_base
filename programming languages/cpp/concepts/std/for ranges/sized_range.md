---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::sized_range` — concept из C++20 из `<ranges>`.

Он проверяет:
> можно ли получить размер range за O(1).

# Главная идея

Range поддерживает:

```cpp
std::ranges::size(r)
```

# Что проверяет concept

Упрощенно:

```cpp
std::ranges::size(r)
```

должен быть valid.

# Реальные sized ranges

|Тип|sized_range|
|---|---|
|`std::vector`|yes|
|`std::array`|yes|
|`std::span`|yes|
|`std::string`|yes|
|`std::forward_list`|no|

# Почему concept полезен

Algorithms могут:
- заранее выделить память
- оптимизировать loops
- избежать repeated traversal
# Пример optimization

```cpp
std::vector<int> out;
out.reserve(std::ranges::size(r));
```

# Где используется
Concept применяется в:
- ranges algorithms
- views
- containers
- lazy pipelines

```cpp
#include <iostream>
#include <vector>
#include <ranges>

struct Range {
    int data[3]{1, 2, 3};

    int* begin() {
        return data;
    }

    int* end() {
        return data + 3;
    }

    std::size_t size() const {
        return 3;
    }
};

template<std::ranges::sized_range T>
void test(const T& _input) {
    std::cout << "Size: " << std::ranges::size(_input) << std::endl;
}

int main() {
    const auto&& vec = std::vector<int>({1, 2, 3, 4, 5});
    test(vec);

    const auto&& range = Range();
    test(range);

    return 0;
}
```

```
Size: 5
Size: 3
```
