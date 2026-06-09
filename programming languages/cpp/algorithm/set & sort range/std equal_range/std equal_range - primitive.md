---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/set & sort range/std equal_range/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    vector<int> numbers {1, 2, 2, 2, 3, 4, 4, 4, 4, 5};
    auto&& range0 = equal_range(numbers.begin(), numbers.end(), 2);
    cout
        << "0) ["
        << distance(numbers.begin(), range0.first)
        << ", " << distance(numbers.begin(), range0.second)
        << "]" << endl;

    reverse(numbers.begin(), numbers.end());
    auto&& range1 = equal_range(
        numbers.begin(), numbers.end(),
        4, greater<int>()
    );
    cout
        << "1) ["
        << distance(numbers.begin(), range1.first)
        << ", " << distance(numbers.begin(), range1.second)
        << "]" << endl;

    return 0;
}
```

```
0) [1, 4]
1) [1, 5]
```
