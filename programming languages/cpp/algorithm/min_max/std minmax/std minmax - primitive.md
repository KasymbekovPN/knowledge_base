---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/min_max/std minmax/_|<=]]

```cpp
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

template<typename K, typename V>
void print_pair(const pair<K, V>&);

int main() {
    const int X00 {10};
    const int X01 {5};
    print_pair(minmax(X00, X01));

    const int X10 {-10};
    const int X11 {5};
    print_pair(minmax(
        X10,
        X11,
        [](int x, int y) { return abs(x) < abs(y); }
    ));

    print_pair(minmax({10, 5, 8, 3, 12}));

    return 0;
}

template<typename K, typename V>
void print_pair(const pair<K, V>& pair) {
    cout
        << "{" << pair.first << ", "
        << pair.second << "}" << endl;
}
```

```
{5, 10}
{5, -10}
{3, 12}
```