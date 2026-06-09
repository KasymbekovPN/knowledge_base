---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/min_max/std max_element/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    const vector<int> NUMBERS {5, 2, 8, 1, 9};

    auto&& it0 = max_element(NUMBERS.begin(), NUMBERS.end());
    cout << "*it0 => " << *it0 << endl;

    auto&& it1 = max_element(
        NUMBERS.begin(),
        NUMBERS.end(),
        [](int x, int y) { return x > y; }
    );
    cout << "*it1 => " << *it1 << endl;

    return 0;
}
```

```
*it0 => 9
*it1 => 1
```