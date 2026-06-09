---
tags:
  - programming-language
  - cpp
  - containers
---
[[_cpp containers span - access|<=]]

__Выполняется без проверки границ__

```cpp
#include <iostream>
#include <vector>
#include <span>

int main(int argc, char const *argv[]) {
    std::vector<int> vec {1, 2, 3};
    std::span<int> s0 {vec};

	try {
	    int val = s.at(100);  // throws std::out_of_range
	} catch (...) {}

    return 0;
}
```
