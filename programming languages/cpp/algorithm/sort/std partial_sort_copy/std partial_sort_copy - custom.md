---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/sort/std partial_sort_copy/_|<=]]

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

int main(int argc, char const *argv[]) {
    const vector<Value> SRC {{5}, {2}, {8}, {1}, {9}, {3}, {7}};
    const size_t SIZE {4};

    vector<Value> numbers (SIZE);
    print_vector(numbers);

    partial_sort_copy(
        SRC.begin(),
        SRC.end(),
        numbers.begin(),
        numbers.end()
    );
    print_vector(numbers);

    partial_sort_copy(
        SRC.begin(),
        SRC.end(),
        numbers.begin(),
        numbers.end(),
        [](const Value& v0, const Value& v1) { return v0.x > v1.x; });
    print_vector(numbers);

    return 0;
}

ostream& operator<<(ostream& os, const Value& value) {
    return os << "{" << value.x << "}";
}

void print_vector(const vector<Value>& container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{{0} {0} {0} {0} }
{{1} {2} {3} {5} }
{{9} {8} {7} {5} }
```

