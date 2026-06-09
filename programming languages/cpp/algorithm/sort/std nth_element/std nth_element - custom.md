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

struct Value {
    int x;
  
    bool operator<(const Value& other) const {
        return x < other.x;
    }
};

ostream& operator<<(ostream&, const Value&);
void print_vector(const vector<Value>&);

int main() {
    vector<Value> values {{5}, {2}, {8}, {1}, {9}, {3}, {7}};
    print_vector(values);

    nth_element(
        values.begin(),
        values.begin() + values.size() / 2,
        values.end()
    );
    print_vector(values);

    nth_element(
        values.begin(),
        values.begin() + values.size() / 2,
        values.end(),
        [](const Value _x, const Value _y) { return _x.x > _y.x; }
    );
    print_vector(values);

    return 0;
}

ostream& operator<<(ostream& _os, const Value& _value) {
    return _os << "{" << _value.x << "}";
}

void print_vector(const vector<Value>& _container) {
    cout << "{";
    for (auto &&item: _container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```


```
{{5} {2} {8} {1} {9} {3} {7} }
{{1} {2} {3} {5} {7} {8} {9} }
{{9} {8} {7} {5} {3} {2} {1} }
```