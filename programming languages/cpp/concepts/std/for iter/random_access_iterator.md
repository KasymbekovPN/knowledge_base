---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::random_access_iterator` — concept из C++20 из `<iterator>`.

Это iterator, который поддерживает:
- чтение
- запись
- `++`
- `--`
- произвольный доступ:
	- `it + n`
    - `it - n`
    - `it[n]`
- сравнение (`<`, `>`, `<=`)

# Главное отличие

- `bidirectional_iterator`: только ++ и --
- `random_access_iterator`: можно прыгать на произвольную позицию

# Реальные random access iterators

|Iterator|Random Access?|
|---|---|
|`vector::iterator`|yes|
|`array::iterator`|yes|
|`deque::iterator`|yes|
|`list::iterator`|no|

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

# Важный момент

Random access iterator НЕ обязан быть contiguous.
Например: `std::deque::iterator` random access,  но память не contiguous.

```cpp
#include <iostream>
#include <iterator>
#include <vector>
#include <concepts>
#include <compare>

struct Iterator {
    using value_type = int;
    using difference_type = std::ptrdiff_t;

    using pointer = int*;
    using reference = int&;

    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;

    int* ptr{nullptr};

    Iterator() = default;

    Iterator(int* _ptr)
        : ptr{_ptr} {}

    reference operator*() const {
        return *ptr;
    }

    pointer operator->() const {
        return ptr;
    }

    reference operator[](difference_type n) const {
        return ptr[n];
    }

    Iterator& operator++() {
        ++ptr;
        return *this;
    }

    Iterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    Iterator& operator--() {
        --ptr;
        return *this;
    }

    Iterator operator--(int) {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    Iterator& operator+=(difference_type n) {
        ptr += n;
        return *this;
    }

    Iterator& operator-=(difference_type n) {
        ptr -= n;
        return *this;
    }

    friend Iterator operator+(
        Iterator it,
        difference_type n
    ) {
        it += n;
        return it;
    }

    friend Iterator operator+(
        difference_type n,
        Iterator it
    ) {
        it += n;
        return it;
    }

    friend Iterator operator-(
        Iterator it,
        difference_type n
    ) {
        it -= n;
        return it;
    }

    friend difference_type operator-(
        const Iterator& a,
        const Iterator& b
    ) {
        return a.ptr - b.ptr;
    }

    bool operator==(const Iterator&) const = default;
    auto operator<=>(const Iterator&) const = default;
};

static_assert(std::random_access_iterator<Iterator>);

template<std::random_access_iterator T>
void test(T it) {
    std::cout << it[2] << std::endl;
}

int main() {
    std::vector<int> v{1, 2, 3, 4};
    test(v.begin());

    int arr[] = {40, 41, 42, 43, 44};
    Iterator it(arr);
    test(it);

    return 0;
}
```

```
3
42
```
