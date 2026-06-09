---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - sort|<=]]

Если нужно сохранить относительный порядок равных элементов, используйте `std::stable_sort`.

```cpp
#include <iostream>
#include <deque>
#include <algorithm>

void _print_deq(const std::deque<std::pair<int, int>>&);

int main() {
    std::deque<std::pair<int, int>> deq = {
        {5, 1}, {3, 2}, {1, 3}, {3, 4}, {2, 5}
    };
    _print_deq(deq);

    std::stable_sort(
        deq.begin(),
        deq.end(),
        [](const auto &a, const auto &b) {return a.first > b.first;}
    );
    _print_deq(deq);

    return 0;
}

void _print_deq(const std::deque<std::pair<int, int>>& deque) {
    std::cout << "{";
    for (auto &pair: deque) {
        std::cout << "(" << pair.first << ", " << pair.second << ") ";
    }
    std::cout << "}" << std::endl;
}
```

```
{(5, 1) (3, 2) (1, 3) (3, 4) (2, 5) }
{(5, 1) (3, 2) (3, 4) (2, 5) (1, 3) }
```
