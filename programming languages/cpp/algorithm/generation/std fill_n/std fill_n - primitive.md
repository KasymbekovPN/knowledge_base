---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/generation/std fill_n/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

template<typename T>
void print_vector(const vector<T>&, const string&&);

int main() {
    vector<int> target;
    fill_n(back_inserter(target), 5, 42);
    print_vector(target, "TARGET");

    return 0;
}

template<typename T>
void print_vector(const vector<T>& _container, const string&& _lbl) {
    cout << "[" << _lbl << "]{";
    string delimiter {""};
    for (auto &&item: _container) {
        cout << delimiter << item;
        delimiter = ", ";
    }
    cout << "}" << endl;
}
```

```
[TARGET]{42, 42, 42, 42, 42}
```
