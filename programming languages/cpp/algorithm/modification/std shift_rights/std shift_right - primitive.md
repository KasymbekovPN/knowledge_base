---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std shift_rights/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    vector<int> vec {10, 20, 30, 40, 50};
    print_vector(vec);

    auto new_end = shift_right(vec.begin(), vec.end(), 2);
    print_vector(vec);

    vec.erase(vec.begin(), new_end);
    print_vector(vec);

    return 0;
}

void print_vector(const vector<int>& container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{10 20 30 40 50 }
{10 20 10 20 30 }
{10 20 30 }
```