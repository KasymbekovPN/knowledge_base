---
tags:
  - programming-language
  - cpp
  - lambda
---
[[programming languages/cpp/lambda/_|<=]]

Хранение лямбда-выражения выполняется:
- через `auto`, локально
- через `std::function`, универсально

```cpp
#include <iostream>
#include <functional>

int main() {
    auto local_lambda = [](int x) -> int {return x + 1;};
    std::function<int(int)> func_lambda = [](int x) -> int {return x - 1;};

    std::cout << "local => " << local_lambda(42) << std::endl;
    std::cout << "func => " << func_lambda(42) << std::endl;

    return 0;
}
```

```
local => 43
func => 41
```
