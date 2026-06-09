---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - find|<=]]

Метод `find()` осуществляет поиск и возвращает итератор.

```cpp
#include <iostream>
#include <map>

void _test_find(const std::map<std::string, int>&, std::string);

int main() {
    std::map<std::string, int> map = {
        {"one", 1},
        {"two", 2},
    };

    _test_find(map, "one");
    _test_find(map, "two");
    _test_find(map, "three");

    return 0;
}

void _test_find(const std::map<std::string, int>& map, std::string key) {
    auto it = map.find(key);
    if (it != map.end()) {
        std::cout
            << "{key: " << key
            << ", value: " << it->second
            << "}" << std::endl;
    } else {
        std::cout
            << "An element with key "
            << key << " does not exist"
            << std::endl;
    }
}
```

```
{key: one, value: 1}
{key: two, value: 2}
An element with key three does not exist
```
