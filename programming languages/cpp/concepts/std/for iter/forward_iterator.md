---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::forward_iterator` — concept из C++20 из `<iterator>`.

Это iterator, который:
- можно читать
- можно увеличивать (`++`)
- можно сравнивать
- поддерживает многократный проход (multi-pass)
# Главное отличие от input_iterator

- `input_iterator` может быть single-pass.
- `forward_iterator` гарантирует: __итератор можно копировать и проходить несколько раз__

# Что проверяет concept

Упрощенно:

```cpp
std::input_iterator<T>
std::incrementable<T>
std::sentinel_for<T, T>
```

и multi-pass guarantee.

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

# Почему vector iterator подходит

`std::vector<int>::iterator`:

- multi-pass
- readable
- incrementable
- equality comparable

# Реальные forward iterators

|Iterator|Forward?|
|---|---|
|`vector::iterator`|yes|
|`list::iterator`|yes|
|`forward_list::iterator`|yes|
|`istream_iterator`|no|

```cpp
#include <iostream>
#include <iterator>
#include <concepts>
#include <vector>

struct CounterIterator {
    using value_tyep = int;
    using difference_type = std::ptrdiff_t;

    int value{0};

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

template<std::forward_iterator T>
void test(T b, T e) {
    for (auto it{b}; it != e; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    for (auto it{b}; it != e; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector v = std::vector({1, 2, 3});
    test(v.begin(), v.end());

    auto c = CounterIterator();
    c++;
    test(c.begin(), c.end());

    return 0;
}
```

```
1 2 3 
1 2 3 
1 
1 
```
