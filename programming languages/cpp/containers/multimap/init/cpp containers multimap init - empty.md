---
tags:
  - programming-language
  - cpp
  - container
  - multimap
---
[[_cpp containers multimap - init|<=]]

```cpp
#include <iostream>
#include <map>

int main() {
    std::multimap<int, int> empty_map;
    std::cout
        << "map size: "
        << empty_map.size()
        << std::endl;

    return 0;
}
```

```
map size: 0
```
