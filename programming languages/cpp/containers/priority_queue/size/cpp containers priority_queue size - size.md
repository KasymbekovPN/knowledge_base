---
tags:
  - programming-language
  - cpp
  - containers
  - priority_queue
---
[[_cpp containers priority_queue - size|<=]]

Метод _size()_ возвращает количество элементов в очереди с приоритетом.

```cpp
#include <iostream>
#include <queue>

void _test_size(std::priority_queue<int>&);

int main() {
    std::priority_queue<int> emptyq;
    _test_size(emptyq);

    std::priority_queue<int> queue;
    queue.push(1);
    _test_size(queue);

    return 0;
}

void _test_size(std::priority_queue<int>& queue) {
    std::cout
        << "Size: "
        << queue.size()
        << std::endl;
}
```

```
Size: 0
Size: 1
```
