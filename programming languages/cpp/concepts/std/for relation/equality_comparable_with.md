---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for relation/_|<=]]

`std::equality_comparable_with<T, U>` — это concept из C++20, который проверяет:

- можно ли сравнивать `T` и `U`
- работают ли:
    - `t == u`
    - `u == t`
    - `t != u`
    - `u != t`
- результат приводится к `bool`

```cpp
#include <iostream>
#include <concepts>

struct Meter {
    int value;
};

struct Kilometer {
    int value;
};

bool operator==(const Meter& m, const Kilometer& k) {
    return m.value == k.value * 1000;
}

bool operator==(const Kilometer& k, const Meter& m) {
    return m.value == k.value * 1000;
}

std::ostream& operator<<(std::ostream& os, const Meter& m) {
    return os << "m{" << m.value << "}";
}

std::ostream& operator<<(std::ostream& os, const Kilometer& k) {
    return os << "k{" << k.value << "}";
}

template<typename T, typename U>
concept EqualityComparable =
requires(const T& t, const U& u) {
    { t == u } -> std::convertible_to<bool>;
    { u == t } -> std::convertible_to<bool>;
};

template<typename T, typename U>
requires EqualityComparable<T, U>
void test(const T& a, const U& b) {
    std::cout
        << a << " == " << b
        << std::boolalpha
        << " :: " << (a == b)
        << '\n';
}

int main() {
    test(42, 42);
    test(3.14159, 3.14159f);
    test(Kilometer{1}, Meter{1000});
}
```

```
42 == 42 :: true
3.14159 == 3.14159 :: false
k{1} == m{1000} :: true
```
