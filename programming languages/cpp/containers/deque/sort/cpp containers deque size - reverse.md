---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - sort|<=]]

Метод _reverse()_ переворачивает порядок элементов в очереди.

```cpp
#include <iostream>
#include <deque>
#include <algorithm>

void _print_deque(const std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 10, 2, 11, 3, 12, 4};
    _print_deque(deq);

    std::reverse(deq.begin(), deq.end());
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
```

```
{1 10 2 11 3 12 4 }
{4 12 3 11 2 10 1 }
```
