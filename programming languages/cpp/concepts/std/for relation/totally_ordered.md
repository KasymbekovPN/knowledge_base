---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for relation/_|<=]]

`std::totally_ordered` — concept из C++20, который проверяет, что тип полностью упорядочен.

То есть для типа работают:
- `==`
- `!=`
- `<`
- `>`
- `<=`
- `>=`

и все сравнения ведут себя согласованно.

```cpp
#include <iostream>
#include <concepts>
#include <compare>

struct NonOrdered {
    int value;

    bool operator==(const NonOrdered&) const = default;
};

struct Ordered {
    int x{};
    int y{};

    auto operator<=>(const Ordered&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const Ordered& _value) {
    return _os << "{x: " << _value.x << ", y: "  << _value.y << "}";
}

template<std::totally_ordered T>
void test(const T& _a, const T& _b) {
    std::cout
        << _a << " == " << _b
        << " :: " << std::boolalpha
        << (_a == _b)
        << std::noboolalpha << std::endl;
}

int main() {
    test(12, 42);
    test(42, 42);
    test(Ordered{1, 2}, Ordered{1, 2});
    test(Ordered{1, 2}, Ordered{1, 42});
    // test(NonOrdered{1}, NonOrdered{1}); // Error

    return 0;
}
```


```
12 == 42 :: false
42 == 42 :: true
{x: 1, y: 2} == {x: 1, y: 2} :: true
{x: 1, y: 2} == {x: 1, y: 42} :: false
```
