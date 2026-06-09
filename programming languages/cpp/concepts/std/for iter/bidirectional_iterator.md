---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::bidirectional_iterator` — concept из C++20 из `<iterator>`.

Это iterator, который умеет:
- читать значения
- двигаться вперед (`++`)
- двигаться назад (`--`)
- поддерживает multi-pass traversal
# Главное отличие

- `forward_iterator`: только ++
- `bidirectional_iterator`: ++ и --

# Реальные bidirectional iterators

|Iterator|Bidirectional?|
|---|---|
|`list::iterator`|yes|
|`set::iterator`|yes|
|`map::iterator`|yes|
|`forward_list::iterator`|no|

# Иерархия iterator concepts

```text
input_iterator
    ↓
forward_iterator
    ↓
bidirectional_iterator
    ↓
random_access_iterator
```

# Разница

| Concept                  | Возможности    |
| ------------------------ | -------------- |
| `forward_iterator`       | только `++`    |
| `bidirectional_iterator` | `++` и `--`    |
| `random_access_iterator` | `+`, `-`, `[]` |

```cpp
#include <iostream>
#include <list>
#include <concepts>

struct CounterIterator {
    using value_type = int;
    using difference_type = std::ptrdiff_t;

    int value{};

    int operator*() const {
        return value;
    }

    CounterIterator& operator++() {
        ++value;
        return *this;
    }

    CounterIterator operator++(int) {
        auto tmp = *this;
        ++(*this);

        return tmp;
    }

    CounterIterator& operator--() {
        --value;
        return *this;
    }

    CounterIterator operator--(int) {
        auto tmp = *this;
        ++(*this);

        return tmp;
    }

    bool operator==(const CounterIterator&) const = default;

    const int* begin() {
        return &value;
    }

    const int* end() {
        return begin() + 1;
    }
};

std::ostream& operator<<(std::ostream& _os, const CounterIterator& _ci) {
    return _os << "{" << _ci.value << "}";
}

template<std::bidirectional_iterator T>
void test(T b, T e) {
    while (b != e) {
        --e;
        std::cout << *e << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::list l = std::list({1, 2, 3});
    test(l.begin(), l.end());

    CounterIterator ci = CounterIterator();
    ++ci;
    test(ci.begin(), ci.end());

    return 0;
}
```

```
3 2 1 
1
```
