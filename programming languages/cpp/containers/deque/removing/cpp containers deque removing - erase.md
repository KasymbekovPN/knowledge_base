---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - removing|<=]]

Метод _erase()_ удаляет элемент или диапазон элементов по итератору.

В C++ вызов метода `erase()` с не валидным итератором приводит к **неопределенному поведению** (undefined behavior)

```cpp
#include <iostream>
#include <string>
#include <deque>

void _print_deque(const std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 2, 3, 4, 5, 7};
    _print_deque(deq);

    auto it = deq.begin();
    std::advance(it, 2);
    deq.erase(it);
    _print_deque(deq);

    auto start_it = deq.begin();
    auto finish_it = deq.begin();
    std::advance(start_it, 2);
    std::advance(finish_it, 5);
    deq.erase(start_it, finish_it);
    _print_deque(deq);

    // UBI, usage invalid iterator
    // it = deq.begin();
    // std::advance(it, 10);
    // deq.erase(it);
    // _print_deque(deq);

    return 0;
}

void _print_deque(const std::deque<int>& deque) {
    std::cout << "{";
    for (auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << "}" << std::endl;
}
```

```
{1 2 3 4 5 7 }
{1 2 4 5 7 }
{1 2 }
```
