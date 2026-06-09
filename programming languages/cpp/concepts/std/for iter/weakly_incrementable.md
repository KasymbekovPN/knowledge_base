---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::weakly_incrementable` — concept из C++20 из `<iterator>`.

Он проверяет, что объект можно увеличивать:
- `++i`
- `i++`

и у него есть `difference_type`.

Используется как базовый concept для iterators.

# Важный момент

Concept НЕ требует equality comparison.

То есть iterator может быть:
- incrementable
- но не comparable

```cpp
#include <iostream>
#include <iterator>
#include <concepts>

struct NonInc {};

struct Inc {
    using difference_type = std::ptrdiff_t;

    int value{};

    Inc& operator++() {
        ++value;
        return *this;
    }

    Inc operator++(int) {
        Inc tmp = *this;
        ++value;
        return tmp;
    }
};

std::ostream& operator<<(std::ostream& _os, const Inc& _value) {
    return _os << "{" << _value.value << "}";
}

template<std::weakly_incrementable T>
void test(T&& _value, const std::string&& _lbl) {
    std::cout << "[" << _lbl << "][before]" << _value << std::endl;
    ++_value;
    std::cout << "[" << _lbl << "][after]" << _value << std::endl;
}

int main() {
    test(42, "primitive");
    test(Inc(), "custom");

    return 0;
}
```

```
[primitive][before]42
[primitive][after]43
[custom][before]{0}
[custom][after]{1}
```
