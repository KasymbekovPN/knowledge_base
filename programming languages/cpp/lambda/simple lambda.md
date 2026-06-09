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
    auto f = [](){std::cout << "Hello" << std::endl;};
    f();

    return 0;
}
```

```
Hello
```
