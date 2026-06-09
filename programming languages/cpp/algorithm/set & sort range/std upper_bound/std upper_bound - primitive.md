---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/set & sort range/std upper_bound/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    vector<int> numbers {1, 3, 5, 7, 9, 11};
    auto&& it0 = upper_bound(numbers.begin(), numbers.end(), 6);
    if (it0 != numbers.end()) {
        cout << "0) " << *it0 << endl;
    }

    reverse(numbers.begin(), numbers.end());
    auto&& it1 = upper_bound(
        numbers.begin(), numbers.end(),
        6, greater<int>()
    );
    if (it1 != numbers.end()) {
        cout << "1) " << *it1 << endl;
    }

    return 0;
}
```

```
0) 7
1) 5
```

