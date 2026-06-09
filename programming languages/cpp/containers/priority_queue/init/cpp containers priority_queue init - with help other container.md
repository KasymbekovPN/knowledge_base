---
tags:
  - programming-language
  - cpp
  - containers
  - priority_queue
---
[[_cpp containers priority_queue - init|<=]]

```cpp
#include <iostream>
#include <vector>
#include <queue>

int main() {
    std::vector<int> v {1, 2, 3};
    std::priority_queue<int> q {v.begin(), v.end()};
    std::cout << "Top: " << q.top() << std::endl;

    return 0;
}
```

```
Top: 3
```
