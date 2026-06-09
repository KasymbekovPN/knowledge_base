---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - addition|<=]]

Метод _insert()_ вставляет элемент в указанную позицию.

В C++ использование несуществующего или недействительного итератора при работе с контейнерами, такими как `std::deque`, приводит к **неопределенному поведению** (undefined behavior).

```cpp
#include <iostream>
#include <string>
#include <deque>

void print_deque(const std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 2, 3};
    print_deque(deq);

    auto it = deq.begin();
    std::advance(it, 1);
    deq.insert(it, 101);
    print_deque(deq);

    deq.insert(deq.end(), 102);
    print_deque(deq);

    // usage invalid iterator
    // it = deq.end();
    // std::advance(it, 10);
    // deq.insert(it, 103);
    // print_deque(deq);

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
1 101 2 3
1 101 2 3 102
```
