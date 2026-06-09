---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/set & sort range/std set_symmetric_difference/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
  
using namespace std;

template<typename T>
void print_vector(const vector<T>&, const string&&);

int main() {
    vector<int> a {1, 2, 3, 4, 5};
    print_vector(a, "A");

    vector<int> b {4, 5, 6, 7, 8};
    print_vector(b, "B");

    vector<int> result0;
    set_symmetric_difference(
        a.begin(), a.end(),
        b.begin(), b.end(),
        back_inserter(result0)
    );
    print_vector(result0, "RESULT0");

    reverse(a.begin(), a.end());
    reverse(b.begin(), b.end());
    vector<int> result1;
    set_symmetric_difference(
        a.begin(), a.end(),
        b.begin(), b.end(),
        back_inserter(result1),
        greater<int>()
    );
    print_vector(result1, "RESULT1");

    return 0;
}

template<typename T>
void print_vector(const vector<T>& _container, const string&& _lbl) {
    cout << "[" << _lbl << "] {";
    string delimiter {""};
    for (auto &&item: _container) {
        cout << delimiter << item;
        delimiter = ", ";
    }
    cout << "}" << endl;
}
```

```
[A] {1, 2, 3, 4, 5}
[B] {4, 5, 6, 7, 8}
[RESULT0] {1, 2, 3, 6, 7, 8}
[RESULT1] {8, 7, 6, 3, 2, 1}
```
