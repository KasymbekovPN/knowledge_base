---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::indirectly_writable` — это concept из C++20, который проверяет:

Он проверяет:
> можно ли записать значение через разыменование (`*obj = value`).

Используется для:
- output iterators
- pointers
- ranges
- algorithms

# Где используется

`std::indirectly_writable` нужен в:
- output iterators
- ranges
- STL algorithms
- `std::copy`
- `std::fill`
- `std::transform`

```cpp
#include <iostream>
#include <concepts>
#include <iterator>
#include <vector>

struct ReadOnlyIterator {
    int value{};

    ReadOnlyIterator(int _value): value{_value} {}

    const int& operator*() const {
        return value;
    }
};

struct WriteIterator {
    int value{};
    WriteIterator(int _value): value{_value} {}

    int& operator*() {
        return value;
    }
};

template<typename T>
requires std::indirectly_writable<T, int>
void test(T _value, std::string&& _lbl) {
    std::cout
        << "[" << _lbl << "][before] "
        << *_value
        << std::endl;
    *_value = 123;
    std::cout
        << "[" << _lbl << "][after] "
        << *_value
        << std::endl;
}

int main() {
    int x{42};
    test(&x, "ptr");

    auto&& v = std::vector<int>({1, 2, 3});
    test(v.begin(), "vec");

    test(WriteIterator(77), "custom");
    // test(ReadOnlyIterator(42), ""); // Error

    return 0;
}
```

```
[ptr][before] 42
[ptr][after] 123
[vec][before] 1
[vec][after] 123
[custom][before] 77
[custom][after] 123
```
