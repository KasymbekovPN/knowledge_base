---
tags:
  - programming-language
  - cpp
  - container
  - set
---
[[_cpp containers set - init|<=]]

```cpp
#include <iostream>
#include <set>

int main() {
    std::set<int> empty_set;
    std::cout
        << "Is it empty? "
        << std::boolalpha
        << empty_set.empty()
        << std::noboolalpha
        << std::endl;
  
    return 0;
}
```

```
Is it empty? true
```
