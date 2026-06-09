---
tags:
  - programming-language
  - cpp
  - functional-objects
  - functor
---
[[programming languages/cpp/functor/_|<=]]

```cpp
#include <iostream>

using std::cout;
using std::endl;

class Adder {

public:
    int operator()(int a, int b) const noexcept{
        return a + b;
    }
};

int main() {
    Adder adder;
    cout << adder(42, 12) << endl;

    return 0;
}
```

```
54
```
