---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::permutable` — concept из C++20 (`<iterator>`), который используется в алгоритмах перестановки элементов (`sort`, `shuffle`, `reverse`, `rotate` и др.).

Он проверяет:
> можно ли через данный итератор читать, записывать и переставлять элементы местами.

# Определение

Упрощённо стандарт определяет его примерно так:

```cpp
template<class I>
concept permutable =
    std::forward_iterator<I> &&
    std::indirectly_movable_storable<I, I> &&
    std::indirectly_swappable<I, I>;
```

# Что требуется

Итератор должен:
### 1. Быть forward_iterator

```cpp
++it;
*it;
```

должны работать.

### 2. Поддерживать перемещение элементов

```cpp
*dst = std::move(*src);
```

### 3. Поддерживать swap элементов

```cpp
std::ranges::iter_swap(it1, it2);
```

# Зачем нужен concept

Алгоритмы вроде:
- `std::ranges::sort`
- `std::ranges::reverse`
- `std::ranges::shuffle`
- `std::ranges::rotate`
меняют порядок элементов.

Им недостаточно только читать значения.

# Итог
`std::permutable<I>` гарантирует, что:
- `I` является `forward_iterator`;
- элементы можно читать;
- элементы можно записывать;
- элементы можно перемещать;
- элементы можно менять местами через `iter_swap`;
- такой итератор подходит для алгоритмов перестановки (`sort`, `reverse`, `shuffle`, `rotate`).

```cpp
#include <iostream>
#include <vector>
#include <iterator>
#include <type_traits>

struct Iterator {
    size_t size;
    int* ptr{nullptr};

    Iterator(int* _ptr, size_t _size):
        ptr{_ptr},
        size(_size) {}

    Iterator& operator++() {
        ptr++;
        return *this;
    }

    Iterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    int* begin() {
        return ptr;
    }

    bool operator==(const Iterator&) const = default;
};

template<std::permutable I>
void test(I _it) {
    std::ranges::iter_swap(_it,  _it + 1);
}

template<typename T>
requires std::same_as<T, std::vector<int>>
void print(T& _input) {
    for (auto& item: _input) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

template<typename T>
requires std::same_as<T, Iterator>
void print(T& _input) {
    for (size_t idx{0}; idx < _input.size; ++idx) {
        std::cout << _input.ptr[idx] << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char const *argv[]) {
    std::vector<int> vec = std::vector<int>({1, 2, 3});
    print(vec);

    int arr[3] {100, 101, 102};
    Iterator iter = Iterator(arr, 3);
    print(iter);

    test(vec.begin());
    print(vec);

    test(iter.begin());
    print(iter);

    return 0;
}
```

```
1 2 3 
100 101 102 
2 1 3 
101 100 102
```
