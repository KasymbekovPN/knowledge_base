---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - list
---
[[_cpp containers list|<=]]

Метод _size()_ возвращает количество элементов в списке.

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> numbers {1, 2, 1, 4, 1, 6, 1};
    std::cout << "size <= " << numbers.size() << std::endl;

    return 0;
}
```

```
size <= 7
```