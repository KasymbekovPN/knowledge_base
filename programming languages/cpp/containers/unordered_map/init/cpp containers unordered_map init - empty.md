---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - init|<=]]

```cpp
#include <iostream>
#include <unordered_map>

int main() {
    std::unordered_map<int, std::string> emap;
    std::cout << "size: " << emap.size() << std::endl;

    return 0;
}
```

```
size: 0
```
