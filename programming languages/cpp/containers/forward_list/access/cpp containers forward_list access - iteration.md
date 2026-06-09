---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - forward_list
---
[[_cpp containers forward_list - access|<=]]

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist {1, 2, 3};

    for (auto it {flist.begin()}; it != flist.end(); it++) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

```
1 2 3 
1 2 3
```