---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/search_n/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _test_search_n(vector<int>&, int, int);

int main() {
    vector<int> data = {1, 2, 3, 4, 4, 4, 5, 6, 4, 4};

    _test_search_n(data, 3, 4);
    _test_search_n(data, 7, 4);

    return 0;
}

void _test_search_n(vector<int>& data, int count, int value) {
    auto it = search_n(data.begin(), data.end(), count, value);

    if (it != data.end()) {
        cout
            << "Found sequence starting at "
            << distance(data.begin(), it)
            << endl;
    } else {
        cout << "Not found" << endl;
    }
}
```

```
Found sequence starting at 3
Not found
```
