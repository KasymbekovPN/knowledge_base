---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/sort/std stable_sort/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Person {
    int age;
    string name;

    bool operator<(const Person& other) const {
        return age < other.age;
    }
};

ostream& operator<<(const ostream&, const Person&);
void print_vector(const vector<Person>&);

int main() {
    vector<Person> people {
        {25, "Alice"},
        {20, "Bob"},
        {25, "Charlie"},
        {20, "Diana"}
    };
    print_vector(people);

    stable_sort(people.begin(), people.end());
    print_vector(people);

    return 0;
}

ostream& operator<<(ostream& os, const Person& person) {
    return os
        << "{age: " << person.age <<
        ", name: " << person.name;
}

void print_vector(const vector<Person>& container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{{age: 25, name: Alice {age: 20, name: Bob {age: 25, name: Charlie {age: 20, name: Diana }
{{age: 20, name: Bob {age: 20, name: Diana {age: 25, name: Alice {age: 25, name: Charlie }
```
