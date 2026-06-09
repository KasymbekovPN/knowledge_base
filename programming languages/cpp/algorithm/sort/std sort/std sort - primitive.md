---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/sort/std sort/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    vector<int> numbers {5, 2, 8, 1, 9};
    print_vector(numbers);

    sort(numbers.begin(), numbers.end());
    print_vector(numbers);

    sort(
        numbers.begin(),
        numbers.end(),
        [](int x, int y) {return x > y;}
    );
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
{5 2 8 1 9 }
{1 2 5 8 9 }
{9 8 5 2 1 }
```

