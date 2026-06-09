---
tags:
  - programming-language
  - cpp
  - container
  - queue
---
[[_cpp containers queue - init|<=]]

```cpp
#include <iostream>
#include <list>
#include <queue>

int main() {
    std::list<int> init {1, 2, 3};
    std::queue<int, std::list<int>> queue{init};
    std::cout << "Front element: " << queue.front() << std::endl;

    return 0;
}
```

```
Front element: 1
```
