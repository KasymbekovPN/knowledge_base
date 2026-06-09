---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/generation/std iota/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <numeric>

using namespace std;

template<typename T>
void print_vector(const vector<T>&, const string&&);

int main() {
    vector<int> target(5);
    iota(target.begin(), target.end(), 100);
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
[TARGET]{100, 101, 102, 103, 104}
```
