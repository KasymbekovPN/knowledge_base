---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/sort/std partial_sort/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main(int argc, char const *argv[]) {
    vector<int> numbers {5, 2, 8, 1, 9, 3, 7};
    print_vector(numbers);

    partial_sort(numbers.begin(), numbers.begin() + 4, numbers.end());
    print_vector(numbers);

    partial_sort(
        numbers.begin(),
        numbers.begin() + 4,
        numbers.end(),
        [](int x, int y) { return x > y; });
    print_vector(numbers);

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
{5 2 8 1 9 3 7 }
{1 2 3 5 9 8 7 }
{9 8 7 5 1 2 3 }
```
