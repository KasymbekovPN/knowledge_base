---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - range|<=]]

```cpp
#include <iostream>
#include <map>

int main() {
    const std::map<std::string, int> map {
        {"a", 0},
        {"b", 1},
        {"c", 2},
        {"d", 4}
    };

    // First element >= "C"
    auto it = map.lower_bound("c");
    std::cout
        << "lb: {" << it->first
        << ", " << it->second
        << "}" << std::endl;

    return 0;
}
```

```
lb: {c, 2}
```
