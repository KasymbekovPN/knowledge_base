---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/sort/std nth_element/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    vector<int> numbers {5, 2, 8, 1, 9, 3, 7};
    print_vector(numbers);

    nth_element(
        numbers.begin(),
        numbers.begin() + numbers.size() / 2,
        numbers.end()
    );
    print_vector(numbers);

    nth_element(
        numbers.begin(),
        numbers.begin() + numbers.size() / 2,
        numbers.end(),
        [](int x, int y) { return x > y; }
    );
    print_vector(numbers);

    return 0;
}

void print_vector(const vector<int>& _container) {
    cout << "{";
    for (auto &&item: _container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```


```
{5 2 8 1 9 3 7 }
{1 2 3 5 7 8 9 }
{9 8 7 5 3 2 1 }
```