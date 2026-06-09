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

void _test_empty(const std::multimap<int, std::string>&);

int main() {
    std::multimap<int, std::string> mmap {
        {1, "one"},
        {1, "one one"},
        {2, "two"},
        {2, "two two"},
        {3, "three"},
        {3, "three three"}
    };
    std::multimap<int, std::string> emap;

    _test_empty(mmap);
    _test_empty(emap);

    return 0;
}

void _test_empty(const std::multimap<int, std::string>& map) {
    std::cout
        << "It it empty? "
        << std::boolalpha
        << map.empty()
        << std::noboolalpha
        << std::endl;
}
```

```
It it empty? false
It it empty? true
```
