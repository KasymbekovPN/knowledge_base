---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - forward_list
---
[[_cpp containers - forward_list|<=]]

Метод _empty()_ проверяет, пуст ли список.

```cpp
#include <iostream>
#include <forward_list>

void is_empty(const std::forward_list<int>&);

int main() {
    std::forward_list<int> empty_flist;
    is_empty(empty_flist);

    std::forward_list<int> flist {1, 2, 3};
    is_empty(flist);

    return 0;
}

void is_empty(const std::forward_list<int>& flist) {
    std::cout
        << "Is empty? "
        << std::boolalpha
        << flist.empty()
        << std::noboolalpha
        << std::endl;
}
```

```
Is empty? true
Is empty? false
```
