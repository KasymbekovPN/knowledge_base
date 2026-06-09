---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _test_mismatch(const vector<int>&, const vector<int>&);

int main() {
    const vector<int> v0 {1, 2, 3, 4, 5};
    const vector<int> v1 {1, 2, 3, 4, 5};
    const vector<int> v2 {1, 2, 9, 4, 5};

    _test_mismatch(v0, v1);
    _test_mismatch(v1, v2);

    return 0;
}

void _test_mismatch(const vector<int>& v0, const vector<int>& v1) {
    auto [it0, it1] = mismatch(v0.begin(), v0.end(), v1.begin());
    if (it0 != v0.end()) {
        cout
            << "Mismatch at position "
            << distance(v0.begin(), it0)
            << " (" << *it0 << ", " << *it1
            << ")" << endl;
    } else {
        cout << "All element match" << endl;
    }
}
```

```
All element match
Mismatch at position 2 (3, 9)
```
