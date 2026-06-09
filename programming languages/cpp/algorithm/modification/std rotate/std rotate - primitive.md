---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std rotate/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
void print_vector(const vector<int>&);

int main() {
    vector<int> vec = {1, 2, 3, 4, 5};
  
    rotate(vec.begin(), vec.end() - 2, vec.end());
    print_vector(vec);

    return 0;
}

void print_vector(const vector<int>& container) {
    cout << "{";
    for (auto &&item : container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{4 5 1 2 3 }
```
