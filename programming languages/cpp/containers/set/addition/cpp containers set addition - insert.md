---
tags:
  - programming-language
  - cpp
  - container
  - set
---
[[_cpp containers set - addition|<=]]

Метод `insert(const T& value)` вставляет элемент в `std::set`. Если элемент уже существует, вставка не происходит. Возвращает пару std::pair<iterator, bool>, где:
- _first_ - итератор на вставленный элемент или на уже существующий элемент.
- _second_ - `true`, если вставка произошла, и `false`, если элемент уже существовал.

```cpp
#include <iostream>
#include <set>

void _print_result(const std::pair<std::set<int>::iterator, bool>&);

int main() {
    std::set<int> set {1, 2, 3};
    _print_result(set.insert(3));
    _print_result(set.insert(4));

    return 0;
}

void _print_result(const std::pair<std::set<int>::iterator, bool>& pair) {
    std::cout
        << *pair.first
        << " <> "
        << std::boolalpha
        << pair.second
        << std::noboolalpha
        << std::endl;
}
```

```
3 <> false
4 <> true
```
