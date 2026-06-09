---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std replace_copy_if/_|<=]]

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
ostream& operator<<(ostream&, const Person&);

int main() {
    const vector<Person> people = {
        {"Alice", 17},
        {"Bob", 25},
        {"Charlie", 15},
        {"Diana", 18}
    };

    vector<Person> dst;
    replace_copy_if(
        people.begin(),
        people.end(),
        back_inserter(dst),
        [](const Person& person) { return person.age >= 18; },
        Person("Adult", 18)
    );
    _print_vector(dst);

    return 0;
}

void _print_vector(const vector<Person>& people) {
    cout << "{";
    for (auto &&person: people) {
        cout << person << " ";
    }
    cout << "}" << endl;
}

ostream& operator<<(ostream& os, const Person& person) {
    return os
        << "{name: " << person.name
        << ", age: " << person.age << "}";
}
```

```
{{name: Alice, age: 17} {name: Adult, age: 18} {name: Charlie, age: 15} {name: Adult, age: 18} }
```
