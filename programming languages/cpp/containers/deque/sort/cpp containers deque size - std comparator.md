---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - sort|<=]]

```cpp
#include <iostream>
#include <deque>
#include <algorithm>

void _print_deque(const std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 10, 2, 11, 3, 12, 4};
    _print_deque(deq);

    std::sort(deq.begin(), deq.end(), std::greater<int>());
    _print_deque(deq);

    std::sort(deq.begin(), deq.end(), std::less<int>());
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
{12 11 10 4 3 2 1 }
{1 2 3 4 10 11 12 }
```