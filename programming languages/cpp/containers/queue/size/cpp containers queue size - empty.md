---
tags:
  - programming-language
  - cpp
  - container
  - queue
---
[[_cpp containers queue - size|<=]]

Метод _empty()_ проверяет, пуста ли очередь.

```cpp
#include <iostream>
#include <queue>
#include <list>

void _test(std::queue<int,std::list<int>>&);

int main() {
    std::queue<int, std::list<int>> emptyq;
    _test(emptyq);

    std::queue<int, std::list<int>> q {std::list<int> {1, 2, 3}};
    _test(q);

    return 0;
}

void _test(std::queue<int, std::list<int>>& queue) {
    std::cout
        << "Is queue empty? "
        << std::boolalpha
        << queue.empty()
        << std::noboolalpha
        << std::endl;
}
```

```
Is queue empty? true
Is queue empty? false
```
