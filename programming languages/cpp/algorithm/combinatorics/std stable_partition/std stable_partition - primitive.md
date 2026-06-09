---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/combinatorics/std stable_partition/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

template<typename T>
void print_vector(const vector<T>&, const string&&);

int main() {
    vector<int> numbers {1, 2, 3, 4, 5, 6, 7, 8};
    print_vector(numbers, "before");

    auto&& it = stable_partition(
        numbers.begin(),
        numbers.end(),
        [](int _x){ return _x %2  == 0; }
    );
    cout << "border value: " << *it << endl;
    print_vector(numbers, "after");

    return 0;
}

template<typename T>
void print_vector(const vector<T>& _container, const string&& _lbl) {
    cout << "[" << _lbl << "] {";
    string delimiter {""};
    for (auto &&item: _container) {
        cout << delimiter << item ;
        delimiter = ", ";
    }
    cout << "}" << endl;
}
```

```
[before] {1, 2, 3, 4, 5, 6, 7, 8}
border value: 1
[after] {2, 4, 6, 8, 1, 3, 5, 7}
```
