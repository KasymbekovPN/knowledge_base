---
tags:
  - programming-language
  - cpp
  - container
  - queue
---
[[_cpp containers queue - addition|<=]]

Добавляет элемент в конец очереди.
```cpp
push(const T& value)
```

```cpp
#include <iostream>
#include <queue>

void _print_size(const std::queue<int>&);

int main() {
    std::queue<int> q;
    _print_size(q);

    q.push(1);
    q.push(42);
    _print_size(q);

    return 0;
}

void _print_size(const std::queue<int>& q) {
    std::cout << "Size: " << q.size() << std::endl;
}
```

```
Size: 0
Size: 2
```