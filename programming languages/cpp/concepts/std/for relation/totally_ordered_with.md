---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for relation/_|<=]]

`std::totally_ordered_with<T, U>` — это concept из C++20, который проверяет:
- можно ли полностью сравнивать `T` и `U`
- работают ли:
    - `==`
    - `!=`
    - `<`
    - `>`
    - `<=`
    - `>=`
- сравнение симметрично
- типы имеют совместимую ordering semantics

```cpp
#include <iostream>
#include <concepts>

struct Meter {
    int value{};
};

struct Kilometer {
    int value{};
};

bool operator==(const Meter& _m, const Kilometer& _k) {
    return _m.value == _k.value * 1000;
}

bool operator==(const Kilometer& _k, const Meter& _m) {
    return _m.value == _k.value * 1000;
}

bool operator<(const Meter& _m, const Kilometer& _k) {
    return _m.value < _k.value * 1000;
}
 
bool operator<(const Kilometer& _k, const Meter& _m) {
    return _k.value * 1000 < _m.value;
}

std::ostream& operator<<(std::ostream& _os, const Meter& _value) {
    return _os << _value.value << "m";
}

std::ostream& operator<<(std::ostream& _os, const Kilometer& _value) {
    return _os << _value.value << "km";
}

template<typename T, typename U>
concept Comparable = requires(const T& t, const U& u) {
    { t == u } -> std::convertible_to<bool>;
    { u == t } -> std::convertible_to<bool>;
    { t < u } -> std::convertible_to<bool>;
    { u < t } -> std::convertible_to<bool>;
};

template<typename T, typename U>
requires Comparable<T, U>
void test(const T& _t, const U& _u) {
    std::cout
        << _t << " == " << _u
        << " :: " << std::boolalpha
        << (_t == _u)
        << std::noboolalpha << std::endl;
}

int main() {
    test(42, 42.0);
    test(Meter{1000}, Kilometer{1});
    test(Kilometer{1}, Meter{777});

    return 0;
}
```

```
42 == 42 :: true
1000m == 1km :: true
1km == 777m :: false
```
