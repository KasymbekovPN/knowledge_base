---
tags:
  - programming-language
  - cpp
  - container
  - queue
---
[[_cpp containers queue - access|<=]]

Метод _back()_ возвращает элемент в конце очереди. Вызов на пустой очереди, это приведет к __неопределенному поведению__.

```cpp
#include <iostream>
#include <queue>

void _test(const std::queue<int>&);

int main() {
    std::queue<int> emptyq;
    _test(emptyq);

    std::queue<int> q;
    q.push(1);
    q.push(42);
    _test(q);

    return 0;
}

void _test(const std::queue<int>& queue) {
    if (queue.size() != 0) {
        std::cout << "Last element: " << queue.back() << std::endl;
    } else {
        std::cout << "It's empty" << std::endl;
    }
}
```

```
It's empty
Last element: 42
```
