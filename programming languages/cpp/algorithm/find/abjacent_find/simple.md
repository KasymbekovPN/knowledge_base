---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/abjacent_find/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _test_adjacent_find(vector<int>&);

int main() {
    vector<int> v0 = {1, 2, 3, 3, 4, 5};
    vector<int> v1 = {1, 2, 3, 4, 5};

    _test_adjacent_find(v0);
    _test_adjacent_find(v1);

    return 0;
}

void _test_adjacent_find(vector<int>& vec) {
    auto it = adjacent_find(vec.begin(), vec.end());
    if (it != vec.end()) {
        cout
            << "Found duplicate at index "
            << distance(vec.begin(), it)
            << ": " << *it << " & " << *(it + 1)
            << endl;
    } else {
        cout << "Not duplicates found" << endl;
    }
}
```

```
Found duplicate at index 2: 3 & 3
Not duplicates found
```
