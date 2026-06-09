---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/combinatorics/std shuffle/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

template<typename T>
void print_vector(const vector<T>&, const string&&);

int main() {
    vector<int> numbers {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    print_vector(numbers, "1");

    random_device rd;
    mt19937 gen(rd());
    shuffle(numbers.begin(), numbers.end(), gen);
    print_vector(numbers, "2");

    shuffle(numbers.begin(), numbers.end(), gen);
    print_vector(numbers, "3");

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
[1] {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
[2] {8, 6, 10, 5, 2, 4, 1, 3, 7, 9}
[3] {8, 4, 9, 1, 3, 10, 6, 7, 5, 2}
```
