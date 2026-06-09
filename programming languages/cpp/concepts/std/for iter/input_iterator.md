---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::input_iterator` — concept из C++20 из `<iterator>`.

Он описывает iterator, из которого можно:
- читать значения
- двигаться вперед (`++`)
- сравнивать
- использовать в однопроходных алгоритмах

# Что умеет input_iterator

Должны работать:

```cpp
*i
++i
i++
i == j
i != j
```

# Что concept проверяет

Упрощенно:

```cpp
std::input_or_output_iterator<T>
std::indirectly_readable<T>
std::incrementable<T>
```

# Главное свойство

`input_iterator` — это iterator для ЧТЕНИЯ.

# Важный нюанс

`input_iterator` может быть:
- single-pass 
- read-only

То есть некоторые input iterators нельзя использовать повторно после прохода.

# Иерархия

```text
weakly_incrementable
    ↓
incrementable
    ↓
input_or_output_iterator
    ↓
input_iterator
```

```cpp
#include <iostream>
#include <vector>
#include <concepts>
#include <iterator>

struct Bad {};

struct CounterIterator {
    using value_type = int;
    using difference_type = std::ptrdiff_t;

    int value{};

    int operator*() const {
        return value;
    }

    CounterIterator& operator++() {
        ++value;
        return *this;
    }

    CounterIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const CounterIterator&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const CounterIterator& _value) {
    return _os << "{" << _value.value << "}";
}

template<std::input_iterator T>
void test(T&& _value, const std::string&& _lbl){
    std::cout << "[" << _lbl << "][before] " << *_value << std::endl;
    ++_value;
    std::cout << "[" << _lbl << "][after] " << *_value << std::endl;
}

int main() {
    test(std::vector<int>({1, 2, 3}).begin(), "primitive");
    test(CounterIterator(), "custom");
    // test(Bad(), ""); // Error

    return 0;
}
```

```
[primitive][before] 1
[primitive][after] 2
[custom][before] 0
[custom][after] 1
```
