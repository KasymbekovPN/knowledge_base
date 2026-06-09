---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for creation/_|<=]]

с++ regular пример

`std::regular` — это concept из C++20, который описывает “обычный” тип:

Тип должен:
- создаваться по умолчанию
- копироваться
- перемещаться
- сравниваться через `==`
- вести себя предсказуемо как обычный value-type

```cpp
#include <iostream>
#include <concepts>

struct NonRegular {
    NonRegular() = default;
    NonRegular(const NonRegular&) = delete;
};

struct Point {
    int x{};
    int y{};

    bool operator==(const Point&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const Point& _p) {
    return _os << "{x: " << _p.x << ", y: " << _p.y << "}";
}

template<std::regular T>
void test(const T&&);

int main() {
    test(42);
    test(std::string("Hello"));
    test(Point());
    // test(NonRegular()); // Error

    return 0;
}

template<std::regular T>
void test(const T&& _input) {
    T copy = _input;
    std::cout << "input: " << _input << std::endl;
    std::cout << "copy: " << copy << std::endl;
}
```

```
input: 42
copy: 42
input: Hello
copy: Hello
input: {x: 0, y: 0}
copy: {x: 0, y: 0}
```
