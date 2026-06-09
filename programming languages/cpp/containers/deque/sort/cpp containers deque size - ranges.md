---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - sort|<=]]

C++20 вводит новый набор библиотек, известных как **Ranges Library**, включая алгоритм `std::ranges::sort`, который предназначен для работы с диапазонами (range-based). Этот алгоритм можно применять ко многим контейнерам стандартной библиотеки, включая `std::deque`.

```cpp
#include <iostream>
#include <deque>
#include <algorithm>
#include <ranges>

void _print_deq(const std::deque<int>&);

int main() {
    std::deque<int> deq = {5, 3, 1, 4, 2};
    _print_deq(deq);

    std::ranges::sort(deq | std::views::take(3));
    _print_deq(deq);

    std::ranges::sort(deq, std::less<int>());
    _print_deq(deq);

    return 0;
}

void _print_deq(const std::deque<int>& deque) {
    for (const auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
5 3 1 4 2 
1 3 5 4 2
1 2 3 4 5
```
