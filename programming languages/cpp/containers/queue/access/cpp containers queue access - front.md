---
tags:
  - programming-language
  - cpp
  - container
  - queue
---
[[_cpp containers queue - access|<=]]

Метод _front()_ возвращает элемент в начале очереди. Вызов на пустой очереди, это приведет к __неопределенному поведению__.

```cpp
#include <iostream>
#include <queue>

void _test(const std::queue<int>&);

int main() {
    std::queue<int> emptyq;
    _test(emptyq);

    std::queue<int> q;
    q.push(1);
    _test(q);

    return 0;
}

void _test(const std::queue<int>& queue) {
    if (queue.size() != 0) {
        std::cout << "First element: " << queue.front() << std::endl;
    } else {
        std::cout << "It's empty" << std::endl;
    }
}
```

```
It's empty
First element: 1
```
