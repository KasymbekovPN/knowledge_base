---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::incrementable` — concept из C++20 из `<iterator>`.

Он проверяет, что объект:
- можно увеличивать:
    - `++i`
    - `i++`
- можно сравнивать через `==`
- является regular-like типом
- post-increment возвращает корректный тип

# Разница

|Concept|Требования|
|---|---|
|`weakly_incrementable`|только increment|
|`incrementable`|increment + equality + regular|

# Почему здесь нужен `operator==`

`incrementable` требует:

```cpp
std::regular<T>
```

а `regular` включает:

```cpp
std::equality_comparable
```

# Важный нюанс

Post-increment должен возвращать объект:

```cpp
i++
```

должен быть valid.
# Где используется

`incrementable` нужен в:
- iterators
- ranges
- STL algorithms
- counting iterators
- views
# Иерархия concepts

```text
weakly_incrementable
    ↓
incrementable
    ↓
input_iterator
```

```cpp
#include <iostream>
#include <concepts>
#include <iterator>

struct NonIncrementable {
    using differnce_type = std::ptrdiff_t;

    NonIncrementable& operator++() {
        return *this;
    }
};

struct Incrementable {
    using difference_type = std::ptrdiff_t;
    int value{};

    Incrementable& operator++() {
        ++value;
        return *this;
    }

    Incrementable operator++(int) {
        auto tmp = *this;
        ++value;
        return tmp;
    }

    bool operator==(const Incrementable&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const Incrementable& _value) {
    return _os << "{" << _value.value << "}";
}

template<std::incrementable T>
void test(T&& _value, const std::string&& _lbl) {
    std::cout << "[" << _lbl << "][before] " << _value << std::endl;
    ++_value;
    std::cout << "[" << _lbl << "][after] " << _value << std::endl;
}

int main() {
    test(42, "primitive");
    test(Incrementable(), "custom");
    // test(NonIncrementable(), ""); // Error

    return 0;
}
```

```
[primitive][before] 42
[primitive][after] 43
[custom][before] {0}
[custom][after] {1}
```
