---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - access|<=]]

Методы _back()_ возвращает последний элемент.

```cpp
#include <iostream>
#include <deque>

template <typename T>
void print_deque(const std::deque<T>&);

int main() {
    std::deque<int> deq {1, 2, 3};
    print_deque(deq);

    std::cout << "Last element: " << deq.back() << std::endl;
    print_deque(deq);

    return 0;
}

template <typename T>
void print_deque(const std::deque<T>& deque) {
    for (auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 3 
Last element: 3
1 2 3
```
