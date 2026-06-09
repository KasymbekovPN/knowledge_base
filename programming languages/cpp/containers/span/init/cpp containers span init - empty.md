---
tags:
  - programming-language
  - cpp
  - containers
---
[[_cpp containers span - init|<=]]

```cpp
#include <iostream>
#include <span>

int main(int argc, char const *argv[]) {
    std::span<int> s;
    std::cout << "size: " << s.size() << std::endl;

    return 0;
}
```

```
size: 0
```
