---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - addition|<=]]

Метод _push_back()_ добавляет элемент в конец очереди.

```cpp
#include <iostream>
#include <string>
#include <deque>

void print_deque(const std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 2, 3};
    print_deque(deq);

    deq.push_back(42);
    print_deque(deq);

    return 0;
}

void print_deque(const std::deque<int>& deque) {
    for (auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 3 
1 2 3 42
```
