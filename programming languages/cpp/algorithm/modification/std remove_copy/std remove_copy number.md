---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std remove_copy/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    const vector<int> SRC {10, 20, 30, 20, 40};
    print_vector(SRC);

    vector<int> dst;
    remove_copy(SRC.begin(), SRC.end(), back_inserter(dst), 20);
    print_vector(SRC);

    return 0;
}

void print_vector(const vector<int>& vec) {
    cout << "{";
    for (auto &&item: vec) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{10 20 30 20 40 }
{10 20 30 20 40 }
```
