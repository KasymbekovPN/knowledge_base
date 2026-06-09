---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - init|<=]]

```cpp
#include <iostream>
#include <map>

using std::cout;
using std::endl;
using std::map;
using std::string;

void _print_map(const map<string, int>&);

int main() {
    const map<string, int> M {
        {"hello", 100},
        {"world", 500},
    };
    _print_map(M);

    return 0;
}

void _print_map(const map<string, int>& m) {
    for (auto &pair: m) {
        cout << "{" << pair.first
            << ", " << pair.second
            << "}" << endl;
    }
}
```

```
{hello, 100}
{world, 500}
```
