---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/min_max/std max/_|<=]]

```cpp
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

int main() {
    const int X00 {10};
    const int X01 {5};
    cout << max(X00, X01) << endl;

    const int x10 {-10};
    const int x11 {5};
    cout
        << max(
            X00,
            X01,
            [](const int x, const int y){ return abs(x) < abs(y); })
        << endl;

    cout << max({10, 5, 8}) << endl;
    cout << max({10, 5, 8}, greater<int>()) << endl;

    return 0;
}
```

```
10
10
10
5
```
