---
tags:
  - programming-language
  - cpp
  - constants
  - constexpr
---
[[programming languages/cpp/constexpr/_|<=]]

```cpp
#include <iostream>

constexpr int square(int _x) { return _x * _x; }

int main() {
    constexpr int val = square(12);
    int runtime_val = square(42);
    std::cout << val << std::endl;
    std::cout << runtime_val << std::endl;

    return 0;
}
```

```
144 
1764
```
