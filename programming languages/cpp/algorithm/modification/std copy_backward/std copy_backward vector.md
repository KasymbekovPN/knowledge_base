---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std copy_backward/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _print_vector(const vector<int>&);

int main() {
    const vector<int> SOURCE {1, 2, 3, 4, 5};
    vector<int> dst(10);
    copy_backward(SOURCE.begin(), SOURCE.end(), dst.end());
    _print_vector(dst);

    return 0;
}

void _print_vector(const vector<int>& v) {
    cout << "{ ";
    for (auto &i: v) {
        cout << i << " ";
    }
    cout << "}" << endl;
}
```

```
{ 0 0 0 0 0 1 2 3 4 5 }
```

