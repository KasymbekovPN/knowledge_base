---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/sort/std is_sorted/_|<=]]

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
void test_is_sorted(const vector<Value>&);

int main() {
    const vector<Value> container1 {{1}, {2}, {3}, {4}, {5}};
    const vector<Value> container2 {{1}, {4}, {5}, {3}, {2}};

    test_is_sorted(container1);
    test_is_sorted(container2);

    return 0;
}

ostream& operator<<(ostream& _os, const Value& _value) {
    return _os << "{" << _value.x << "}";
}

void test_is_sorted(const vector<Value>& _container) {
    if (is_sorted(_container.begin(), _container.end())) {
        cout << "It is sorted in ascending order" << endl;
    } else {
        cout << "It is not sorted" << endl;
    }
}
```

```
It is sorted in ascending order
It is not sorted
```
