---
tags:
  - programming-language
  - cpp
  - container
  - multimap
---
[[_cpp containers multimap - size|<=]]

```cpp
#include <iostream>
#include <map>

int main() {
    std::multimap<int, std::string> mmap {
        {1, "one"},
        {1, "one one"},
        {2, "two"},
        {2, "two two"},
        {3, "three"},
        {3, "three three"}
    };
    std::cout
        << "size: "
        << mmap.size()
        << std::endl;

    return 0;
}
```

```
size: 6
```
