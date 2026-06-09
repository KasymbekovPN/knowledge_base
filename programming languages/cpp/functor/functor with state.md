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

class Accumulator {

private:
    static const int DEFAULT_VALUE = 0;
    int value;

public:
    Accumulator() noexcept: value{DEFAULT_VALUE} {}

    int operator()(int x) noexcept {
        return value += x;
    }

    int get() const noexcept {
        return value;
    }

    int reset() noexcept {
        int copied_value = value;
        value = DEFAULT_VALUE;

        return copied_value;
    }
};

int main() {
    Accumulator acc;
    cout << acc.get() << endl;
    cout << acc(12) << endl;
    cout << acc(42) << endl;
    cout << acc.reset() << endl;
    cout << acc.get() << endl;

    return 0;
}
```

```
0
12
54
54
0
```
