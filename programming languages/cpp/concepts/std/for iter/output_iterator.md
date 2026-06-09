---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::output_iterator` — concept из C++20 из `<iterator>`.

Он описывает iterator, в который можно ЗАПИСЫВАТЬ данные.

# Что должен уметь output_iterator

Должны работать операции:

```cpp
*i = value
++i
i++
```

# Что проверяет concept

Упрощенно:

```cpp
std::input_or_output_iterator<T>
std::indirectly_writable<T, Value>
```

# Важный нюанс

Output iterators часто:
- single-pass
- write-only

После записи читать может быть нельзя.

# Иерархия iterator concepts

```text
weakly_incrementable
    ↓
input_or_output_iterator
    ↓
output_iterator
```

# Важный момент

`output_iterator` — это iterator для записи. Чтение не обязательно.

```cpp
#include <iostream>
#include <concepts>
#include <vector>
#include <iterator>

struct Reader {
    int operator*() {
        return 0;
    }
};

struct Writer {
    using difference_type = std::ptrdiff_t;

    int value{};

    Writer& operator*() {
        return *this;
    }

    Writer& operator=(int _value) {
        value = _value;
        return *this;
    }

    Writer& operator++() {
        value++;
        return *this;
    }

    Writer& operator++(int) {
        value++;
        return *this;
    }
};

std::ostream& operator<<(std::ostream& _os, const Writer& _w) {
    return _os << "{" << _w.value << "}";
}

template<std::output_iterator<int> T>
void test(T&& _value, const std::string&& _lbl) {
    std::cout << "[" << _lbl << "][before] " << *_value << std::endl;
    ++_value;
    std::cout << "[" << _lbl << "][after] " << *_value << std::endl;
}

int main() {
    test(std::vector({1, 2, 3}). begin(), "primitive");
    test(Writer(), "custom");
    // test(Reader(), ""); // Error

    return 0;
}
```

```
[primitive][before] 1
[primitive][after] 2
[custom][before] {0}
[custom][after] {1}
```
