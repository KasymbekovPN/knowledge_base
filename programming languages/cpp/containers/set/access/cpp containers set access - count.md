---
tags:
  - programming-language
  - cpp
  - container
  - set
---
[[_cpp containers set - access|<=]]

Метод `count(const T& value)` возвращает количество элементов с указанным значением (0 или 1, так как элементы уникальны).

```cpp
#include <iostream>
#include <set>

int main() {
    std::set<int> s {1, 2, 3};
    std::cout << s.count(2) << std::endl;
    std::cout << s.count(42) << std::endl;

    return 0;
}
```

```
1
0
```