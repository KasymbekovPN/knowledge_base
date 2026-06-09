---
tags:
  - programming-language
  - cpp
  - containers
  - priority_queue
---
[[_cpp containers priority_queue - addition|<=]]

Метод _pop()_ удаляет элемент с наивысшим приоритетом (вершину очереди). Вызов _pop()_ на пустой очереди приводит к неопределенному поведению.

```cpp
#include <iostream>
#include <queue>
#include <vector>

void _test_pop(std::priority_queue<int, std::vector<int>>&);

int main() {
    std::priority_queue<int, std::vector<int>> q;
    q.push(1);
    q.push(2);

    for (size_t i {}; i < 3; i++) {
        _test_pop(q);
    }

    return 0;
}

void _test_pop(std::priority_queue<int, std::vector<int>>& queue) {
    if (queue.size() > 0) {
        queue.pop();
        std::cout << "Size: " << queue.size();
    } else {
        std::cout << "Empty!";
    }
    std::cout << std::endl;
}
```

```
Size: 1
Size: 0
Empty!
```
