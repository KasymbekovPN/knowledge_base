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
    int a {42};
    int b {12};

    auto f = [a, &b]() {
        b++;
        std::cout << "{" << a << ", " << b << "}" << std::endl;
    };
    f();

    std::cout << "{" << a << ", " << b << "}" << std::endl;

    return 0;
}
```

```
{42, 13}
{42, 13}
```
