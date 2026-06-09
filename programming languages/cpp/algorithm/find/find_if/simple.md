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

bool _is_even(const int);
void _test_find_if(const vector<int>&);

bool _is_even(const int value) {
    return value % 2 == 0;
}

int main() {
    const vector<int> v0 {1, 2, 3, 4, 5};
    const vector<int> v1 {4, 5, 6};
    const vector<int> v2 {1, 3, 5, 7, 9};

    _test_find_if(v0);
    _test_find_if(v1);
    _test_find_if(v2);

    return 0;
}

void _test_find_if(const vector<int>& vec) {
    auto it = find_if(vec.begin(), vec.end(), _is_even);
    if (it != vec.end()) {
        cout
            << "First even element " << *it
            <<  " at position " << (it - vec.begin())
            << endl;
    } else {
        cout << "No even numbers found" << endl;
    }
}
```

```
First even element 2 at position 1
First even element 4 at position 0
No even numbers found
```

---



---

---
## ✅ Пример 3: поиск по пользовательскому типу данных

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

struct Person {
    std::string name;
    int age;
};

int main() {
    std::vector<Person> people = {
        {"Alice", 25},
        {"Bob", 17},
        {"Charlie", 19},
        {"Diana", 16}
    };

    auto it = std::find_if(people.begin(), people.end(), [](const Person& p) {
        return p.age < 18; // Ищем первого несовершеннолетнего
    });

    if (it != people.end())
        std::cout << "Underage: " << it->name << ", " << it->age << "\n";
    else
        std::cout << "All adults\n";
}
```

### Вывод:
```
Underage: Bob, 17
```

---


---
---



---
