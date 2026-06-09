---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - forward_list
---
[[_cpp containers forward_list - access|<=]]

Метод _front()_ возвращает первый элемент списка.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist {1, 2, 3};
    std::cout
        << "First element: "
        << flist.front()
        << std::endl;

    return 0;
}
```

```
First element: 1
```