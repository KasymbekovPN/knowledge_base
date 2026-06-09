---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/find/_|<=]]

> 💡 Чтобы `std::find` работал с пользовательскими типами, нужно определить `operator==`.

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

void _test_find(const vector<Person>&, const Person&);

int main() {
    const vector<Person> people {
        {"Alice", 20},
        {"Bob", 25},
        {"Tom", 30}
    };
    const vector<Person> persons {
        {"Bob", 20},
        {"Bob", 25},
        {"Bob", 30}
    };
    for(auto& person: persons) {
        _test_find(people, person);
    }

    return 0;
}

void _test_find(const vector<Person>& persons, const Person& person) {
    auto it = std::find(persons.begin(), persons.end(), person);
    if (it != persons.end()) {
        cout << "Found" << endl;
    } else {
        cout << "Not found" << endl;
    }
}
```

```
Not found
Found
Not found
```
