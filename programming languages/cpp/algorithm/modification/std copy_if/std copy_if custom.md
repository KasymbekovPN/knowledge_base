---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std copy_if/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

struct Person {
    string name;
    unsigned age;
};

void _print_vector(const vector<Person>&);

int main() {
    const vector<Person> SOURCE = {
        {"Alice", 25},
        {"Bob", 17},
        {"Charlie", 19},
        {"Diana", 16}
    };
    vector<Person> dst;
    copy_if(
        SOURCE.begin(),
        SOURCE.end(),
        back_inserter(dst),
        [](const Person& p) { return p.age >= 18; }
    );
    _print_vector(dst);

    return 0;
}

void _print_vector(const vector<Person>& v) {
    cout << "{ ";
    for (auto &&p: v) {
        cout << "{name: " << p.name << ", age: " << p.age << "}";
    }
    cout << "}" << endl;
}
```

```
{ {name: Alice, age: 25}{name: Charlie, age: 19}}
```
