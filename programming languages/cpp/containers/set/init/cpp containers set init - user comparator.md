---
tags:
  - programming-language
  - cpp
  - container
  - set
---
[[_cpp containers set - init|<=]]

```cpp
#include <iostream>
#include <set>

template<typename T, typename C>
void _print_set(const std::set<T, C>&);

int main() {
    std::set<int, std::greater<int>> set {4, 1, 3, 1, 4, 7};
    _print_set(set);

    auto comparator = [](int a, int b) {return a < b;};
    std::set<int, decltype(comparator)> cset {
	    {4, 1, 3, 1, 4, 7},
	    comparator
	};
    _print_set(cset);
  
    return 0;
}

template<typename T, typename C>
void _print_set(const std::set<T, C> &set) {
    for (auto &item: set) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
7 4 3 1 
1 3 4 7
```
