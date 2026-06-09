---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - size|<=]]

Метод _empty()_ проверяет, пуста ли очередь.

```cpp
#include <iostream>
#include <deque>

void _print_empty(const std::deque<int>&);

int main() {
    std::deque<int> empty_deq {};
    std::deque<int> deq {1, 2, 3};
  
    _print_empty(empty_deq);
    _print_empty(deq);

    return 0;
}

void _print_empty(const std::deque<int>& deque) {
    std::cout
        << "It's empty - "
        << std::boolalpha
        << deque.empty()
        << std::noboolalpha
        << std::endl;
}
```

```
It's empty - true
It's empty - false
```
