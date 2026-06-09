---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - access|<=]]

Если элемент не существует, то он создается.

```cpp
#include <iostream>
#include <map>
#include <string>

void _print_item(std::map<std::string, int>&, std::string);

int main() {
    std::map<std::string, int> map = {
        {"one", 1},
        {"two", 2},
    };
  
    _print_item(map, "one");
    _print_item(map, "two");
    _print_item(map, "three");

    return 0;
}

void _print_item(std::map<std::string, int>& map, std::string key) {
    std::cout
        << "{key: " << key
        << ", value: " << map[key]
        << "}" << std::endl;
}
```

```
{key: one, value: 1}
{key: two, value: 2}
{key: three, value: 0}
```
