---
tags:
  - programming-language
  - cpp
  - containers
  - priority_queue
---
[[_cpp containers priority_queue - addition|<=]]

Метод _push(const T& value)_ добавляет элемент в очередь с приоритетом.

```cpp
#include <iostream>
#include <queue>

int main() {
    std::priority_queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    std::cout << "size: " << q.size() <<  std::endl;

    return 0;
}
```

```
size: 3
```
