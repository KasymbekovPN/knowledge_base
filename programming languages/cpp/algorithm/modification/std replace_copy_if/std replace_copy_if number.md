---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std replace_copy_if/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

void _print_vector(vector<int>&);

int main() {
    const vector<int> SRC {1, 2, 3, 4, 5};

    vector<int> dst;
    replace_copy_if(
        SRC.begin(),
        SRC.end(),
        back_inserter(dst),
        [](int x) { return x % 2 == 0; },
        42
    );
    _print_vector(dst);

    return 0;
}

void _print_vector(vector<int>& vec) {
    cout << "{";
    for (auto &i: vec) {
        cout << i << " ";
    }
    cout << "}" << endl;
}
```

```
{1 42 3 42 5 }
```
