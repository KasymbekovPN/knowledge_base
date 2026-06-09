---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/min_max/std min_element/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    const vector<int> NUMBERS {5, 2, 8, 1, 9};

    auto&& it0 = min_element(NUMBERS.begin(), NUMBERS.end());
    cout << "*it0 => " << *it0 << endl;

    auto&& it1 = min_element(
        NUMBERS.begin(),
        NUMBERS.end(),
        [](int x, int y) { return x > y; }
    );
    cout << "*it1 => " << *it1 << endl;

    return 0;
}
```

```
*it0 => 1
*it1 => 9
```
