---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std remove/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _print_vector(const vector<int>&);

int main() {
    vector<int> vec {1, 2, 3, 2, 4, 2, 5};
    _print_vector(vec);

    auto new_end = remove(vec.begin(), vec.end(), 2);
    _print_vector(vec);

    vec.erase(new_end, vec.end());
    _print_vector(vec);

    return 0;
}

void _print_vector(const vector<int>& vec) {
    cout << "{";
    for (auto &&item: vec) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{1 2 3 2 4 2 5 }
{1 3 4 5 4 2 5 }
{1 3 4 5 }
```

