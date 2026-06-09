---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/sort/std is_sorted/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _test_is_sorted(const vector<int>&);

int main() {
    const vector<int> container1 {1, 2, 3, 4, 5};
    const vector<int> container2 {1, 4, 5, 3, 2};

    _test_is_sorted(container1);
    _test_is_sorted(container2);

    return 0;
}

void _test_is_sorted(const vector<int>& _container) {
    if (is_sorted(_container.begin(), _container.end())) {
        cout << "It is sorted in ascending order" << endl;
    } else {
        cout << "It is not sorted" << endl;
    }
}
```

```
It is sorted in ascending order
It is not sorted
```
