---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::sortable` — concept из C++20 (`<iterator>`), который используется алгоритмами сортировки:

- `std::ranges::sort`
- `std::ranges::stable_sort`
- `std::ranges::partial_sort`
- `std::ranges::nth_element`

Он проверяет:
> можно ли переставлять элементы диапазона и сравнивать их в заданном порядке.

# Определение

Упрощённо:

```cpp
template<
    class I,
    class Comp = std::ranges::less,
    class Proj = std::identity>
concept sortable =
    std::permutable<I> &&
    std::indirect_strict_weak_order<
        Comp,
        std::projected<I, Proj>
    >;
```

---

# Из чего состоит sortable

## 1. `std::permutable<I>`

Требует:

```cpp
std::ranges::iter_swap(it1, it2);
```

и

```cpp
*it = value;
```

То есть элементы можно менять местами и перемещать.

## 2. `std::indirect_strict_weak_order`

Требует возможность сравнения элементов.

Например:

```cpp
comp(*it1, *it2);
```

должно быть корректно.

# Итог

`std::sortable<I, Comp, Proj>` гарантирует, что:
- элементы доступны через итератор `I`;
- элементы можно переставлять местами;
- элементы можно перемещать и записывать обратно;
- элементы можно сравнивать через компаратор `Comp`;
- при необходимости можно применять проекцию `Proj`;
- такие итераторы подходят для алгоритмов сортировки и упорядочивания.

```cpp
#include <iostream>
#include <algorithm>
#include <ranges>
#include <vector>

struct Buffer {
    size_t size{};
    int* data{nullptr};

    Buffer(int* _data, size_t _size):
        data{_data},
        size(_size) {}

    Buffer& operator++() {
        data++;

        return *this;
    }

    Buffer operator++(int) {
        auto tmp = *this;
        ++(*this);

        return tmp;
    }

    int* begin() {
        return data;
    }

    int* end() {
        return data + size;
    }

    bool operator==(const Buffer&) const = default;
    auto operator<=>(const Buffer&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const Buffer& _buffer) {
    auto&& delimiter = std::string("");
    _os << "{";
    for (size_t i{}; i < _buffer.size; ++i) {
        _os << delimiter << _buffer.data[i];
        delimiter = " ";
    }
    std::cout << "}";

    return _os;
}

template<std::sortable T>
void test(T&& _begin, T&& _end) {
    std::ranges::sort(_begin, _end);
}

int main() {
    std::vector<int> vec{2, 1, 5, 4, 2};
    for (int item: vec) { std::cout << item << " "; }
    std::cout << std::endl;

    test(vec.begin(), vec.end());
    for (int item: vec) { std::cout << item << " "; }
    std::cout << std::endl;

    const size_t SIZE{5};
    int arr[SIZE] {8, 9, 7, 5, 1};
    auto&& buffer = Buffer(arr, SIZE);
    std::cout << buffer << std::endl;

    test(buffer.begin(), buffer.end());
    std::cout << buffer << std::endl;

    return 0;
}
```

```
2 1 5 4 2 
1 2 2 4 5 
{8 9 7 5 1}
{1 5 7 8 9}
```
