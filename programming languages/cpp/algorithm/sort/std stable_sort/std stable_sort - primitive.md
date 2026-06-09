---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/sort/std stable_sort/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    vector<int> numbers = {3, 1, 4, 1, 5};
    print_vector(numbers);

    stable_sort(numbers.begin(), numbers.end());
    print_vector(numbers);

    return 0;
}

void print_vector(const vector<int>& container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item <<  " ";
    }
    cout << "}" << endl;
}
```

```
{3 1 4 1 5 }
{1 1 3 4 5 }
```
