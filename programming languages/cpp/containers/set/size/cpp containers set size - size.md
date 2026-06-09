---
tags:
  - programming-language
  - cpp
  - container
  - set
---
[[_cpp containers set - size|<=]]

Метод `size()` возвращает количество элементов в std::set.

```cpp
#include <iostream>
#include <set>

void _test_size(const std::set<int>&);

int main() {
    std::set<int> empty_set;
    _test_size(empty_set);

    std::set<int> s1 {1};
    _test_size(s1);

    std::set<int> s3 {1, 2, 3};
    _test_size(s3);

    return 0;
}

void _test_size(const std::set<int>& set) {
    std::cout << "size: " << set.size() << std::endl;
}
```

```
size: 0
size: 1
size: 3
```
