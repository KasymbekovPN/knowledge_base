---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - removing|<=]]

Метод _pop_back()_ удаляет последний элемент.

В C++ вызов метода `pop_back()` на пустом `std::deque` приводит к **неопределенному поведению** (undefined behavior)

```cpp
#include <iostream>
#include <string>
#include <deque>

void _print_deque(const std::deque<int>&);
void _pop_back(std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 2};
    _print_deque(deq);

    _pop_back(deq);
    _print_deque(deq);

    _pop_back(deq);
    _print_deque(deq);

    _pop_back(deq);
    _print_deque(deq);

    return 0;
}

void _print_deque(const std::deque<int>& deque) {
    std::cout << "{";
    for (auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << "}" << std::endl;
}

void _pop_back(std::deque<int>& deque) {
    if (!deque.empty()) {
        deque.pop_back();
    } else {
        std::cout << "Empty!" << std::endl;
    }
}
```

```
{1 2 }
{1 }
{}
Empty!
{}
```
