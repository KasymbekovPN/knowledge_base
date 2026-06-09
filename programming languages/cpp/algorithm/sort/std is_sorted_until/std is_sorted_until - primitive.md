---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/sort/std is_sorted_until/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void test(const vector<int>&, const string&);

int main() {
    const vector<int> NOT_SORTED {1, 2, 3, 5, 4, 6};
    const vector<int> SORTED {1, 2, 3, 4, 5, 6};

    test(NOT_SORTED, "NOT_SORTED");
    test(SORTED, "SORTED");

    return 0;
}

void test(const vector<int>& container, const string& lbl) {
    cout << lbl << endl;

    auto&& it = is_sorted_until(container.begin(), container.end());
    if (it == container.end()) {
        cout << "The entire range is sorted" << endl;
        return;
    }

    cout
        << "Sorted until index: "
        << distance(container.begin(), it) << endl
        << "First unsorted: " << *it <<  endl;
}
```

```
NOT_SORTED
Sorted until index: 4
First unsorted: 4
SORTED
The entire range is sorted
```
