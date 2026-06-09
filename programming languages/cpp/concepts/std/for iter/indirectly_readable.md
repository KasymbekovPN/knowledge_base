---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::indirectly_readable` — это concept из C++20, который проверяет:

Этот concept проверяет:
> можно ли “читать” значение через разыменование (`*obj`).

Обычно используется для:
- iterators
- pointers
- smart iterators
- ranges

# Что проверяет concept

Упрощенно:
```cpp
*obj
```

должно быть валидно.
И должны существовать связанные типы:
- `value_type`
- `reference`
- `iter_value_t`
- `iter_reference_t`

```cpp
#include <iostream>
#include <concepts>
#include <vector>
#include <iterator>

struct NonIterator {};

struct CustomIterator {
    using value_type = int;
    using difference_type = std::ptrdiff_t;

    int value{};

    CustomIterator(int _value): value{_value} {};

    const int& operator*() const {
        return value;
    }
};

template<std::indirectly_readable T>
void test(const T& _it) {
    std::cout << *_it << std::endl;
}

int main() {
    int x{42};
    test(&x);

    auto&& v = std::vector<int>({1, 2, 3});
    test(v.begin());

    test(CustomIterator(42));

    // test(NonIterator()); // Error

    return 0;
}
```


```
42
1
42
```
