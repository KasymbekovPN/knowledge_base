---
tags:
  - programming-language
  - cpp
  - lambda
---
[[programming languages/cpp/lambda/_|<=]]

```cpp
#include <iostream>

int main() {
    int x {42};

    auto f = [=]() {
		// cannot assign to a variable captured by copy
		// in a non-mutable lambda
        // x += 1;
        std::cout << "Inner 0: " << x << std::endl;
    };
    f();

    auto m = [=]() mutable {
        x += 1;
        std::cout << "Inner 1: " << x << std::endl;
    };
    m();

    std::cout << "Outer: " << x << std::endl;

    return 0;
}
```

```
Inner 0: 42
Inner 1: 43
Outer: 42
```
