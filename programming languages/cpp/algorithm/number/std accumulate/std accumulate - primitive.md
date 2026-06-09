---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/number/std accumulate/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <numeric>

using namespace std;

int main() {
    const vector<int> NUMBERS {1, 2, 3, 4, 5};
    cout
        << accumulate(NUMBERS.begin(), NUMBERS.end(), 0)
        << endl;
    cout
        << accumulate(
            NUMBERS.begin(),
            NUMBERS.end(),
            1,
            multiplies<int>()
        )
        << endl;
    cout
        << accumulate(
            NUMBERS.begin(),
            NUMBERS.end(),
            0,
            [](const int a, const int b) {
                return a - b;
            }
        )
        << endl;

    return 0;
}
```

```
15
120
-15
```
