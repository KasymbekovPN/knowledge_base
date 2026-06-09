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
#include <vector>

using namespace std;

template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N-1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

int main() {
    cout << Factorial<0>::value << endl;
    cout << Factorial<1>::value << endl;
    cout << Factorial<2>::value << endl;
    cout << Factorial<3>::value << endl;
    cout << Factorial<4>::value << endl;

    return 0;
}
```

```
1
1
2
6
24
```
