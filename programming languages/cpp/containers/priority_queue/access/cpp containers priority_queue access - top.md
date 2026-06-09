---
tags:
  - programming-language
  - cpp
  - containers
  - priority_queue
---
[[_cpp containers priority_queue - access|<=]]

Метод _top()_ возвращает элемент с наивысшим приоритетом (вершину очереди). Вызов _top()_ на пустой очереди приводит к неопределенному поведению.

```cpp
#include <iostream>
#include <queue>
#include <vector>

void _test_top(const std::priority_queue<int>&);

int main() {
    const std::vector<int> vector{3, 7, 1};
    const
    std::priority_queue<int, std::vector<int>> queue {
	    vector.begin(),
	    vector.end()};
    _test_top(queue);

    return 0;
}

void _test_top(const std::priority_queue<int>& queue) {
    if (queue.size() > 0){
        std::cout << "Top: " << queue.top();
    } else {
        std::cout << "empty!";
    }
    std::cout << std::endl;
}
```

```
Top: 7
```