---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for relation/_|<=]]

`std::equality_comparable` — это concept из C++20, который проверяет:

- можно ли сравнивать объекты через `==`
- можно ли сравнивать через `!=`
- результат сравнения приводится к `bool`

```cpp
#include <iostream>
#include <concepts>

struct NonComparable {
    int x{};
};

struct Comparable {
    int x{};
    int y{};

    bool operator==(const Comparable&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const Comparable& _input) {
    return _os << "{x: " << _input.x << ", y: " << _input.y << "}";
}

template<std::equality_comparable T>
void test(const T&&, const T&&);

int main() {
    test(42, 42);
    test(2.72, 1.414);
    test(Comparable(), Comparable());
    // test(NonComparable(), NonComparable()); // Error

    return 0;
}

template<std::equality_comparable T>
void test(const T&& _input0, const T&& _input1) {
    std::cout
        << std::boolalpha
        << _input0 << " == " << _input1
        << " :: " << (_input0 == _input1)
        << std::noboolalpha
        << std::endl;
}
```

```
42 == 42 :: true
2.72 == 1.414 :: false
{x: 0, y: 0} == {x: 0, y: 0} :: true
```
