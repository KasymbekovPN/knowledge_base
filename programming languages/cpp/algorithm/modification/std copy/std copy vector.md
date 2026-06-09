---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/_|<=]]

```cpp
#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

void _print_vector(const vector<int>&);

int main() {
    const vector<int> SOURCE {1, 2, 3, 4, 5};

    vector<int> dst(SOURCE.size());
    copy(SOURCE.begin(), SOURCE.end(), dst.begin());
    _print_vector(dst);

    return 0;
}

void _print_vector(const vector<int>& v) {
    cout << "{";
    for (auto &item: v) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{1 2 3 4 5 }
```
