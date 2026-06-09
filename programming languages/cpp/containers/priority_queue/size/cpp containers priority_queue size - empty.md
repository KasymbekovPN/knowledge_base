---
tags:
  - programming-language
  - cpp
  - containers
  - priority_queue
---
[[_cpp containers priority_queue - size|<=]]

Метод _empty()_ проверяет, пуста ли очередь с приоритетом.

```cpp
#include <iostream>
#include <queue>

void _test_empty(std::priority_queue<int>&);

int main() {
    std::priority_queue<int> emptyq;
    _test_empty(emptyq);

    std::priority_queue<int> queue;
    queue.push(1);
    _test_empty(queue);

    return 0;
}

void _test_empty(std::priority_queue<int>& queue) {
    std::cout
        << "Is it empty? "
        << std::boolalpha
        << queue.empty()
        << std::noboolalpha
        << std::endl;
}
```

```
Is it empty? true
Is it empty? false
```
