---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std unique copy/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    const vector<int> SRC = {1, 1, 2, 2, 2, 3, 4, 4};
    print_vector(SRC);

    vector<int> dst0;
    unique_copy(SRC.begin(), SRC.end(), back_inserter(dst0));
    print_vector(dst0);

    vector<int> dst1;
    unique_copy(
        SRC.begin(),
        SRC.end(),
        back_inserter(dst1),
        [](int a, int b) {return b - a == 1;}
    );
    print_vector(dst1);

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
{1 1 2 2 2 3 4 4 }
{1 2 3 4 }
{1 1 3 }
```
