---
tags:
  - programming-language
  - cpp
  - lambda
---
[[programming languages/cpp/lambda/_|<=]]

> ✅ Безопасно, если переменная захвачена по значению.

```cpp
#include <iostream>
#include <thread>

int main() {
    const int VALUE {42};
    std::thread t([VALUE](){
        std::cout << "Value => " << VALUE << std::endl;
    });
    t.join();

    return 0;
}
```

```
Value => 42
```
