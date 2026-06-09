---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Person {
    string name;
    unsigned age;

    bool operator==(const Person& other) const {
        return name == other.name && age == other.age;
    }
};

bool _check(const Person&);
void _test_find_if_not(const vector<Person>&);

int main() {
    const vector<Person> v0 {
        {"Bob", 17},
        {"Tom", 19},
        {"Mark", 16}
    };
    const vector<Person> v1 {
        {"Bob", 17},
        {"Tom", 17},
        {"Mark", 16}
    };

    _test_find_if_not(v0);
    _test_find_if_not(v1);

    return 0;
}

bool _check(const Person& person) {
    return person.age < 18;
}

void _test_find_if_not(const vector<Person>& vec) {
    auto it = find_if_not(vec.begin(), vec.end(), _check);
    if (it != vec.end()) {
        cout
            << "Found " << it->name
            <<  " at position " << (it - vec.begin())
            << endl;
    } else {
        cout << "Not found" << endl;
    }
}
```

```
Found Tom at position 1
Not found
```
