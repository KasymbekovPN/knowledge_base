---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - deleting|<=]]

Метод `clear` удаляет все элементы

```cpp
#include <iostream>
#include <unordered_map>

int main() {
    std::unordered_map<std::string, int> map {
        {"one", 1},
        {"two", 2},
        {"three", 3},
        {"four", 4},
        {"five", 5}
    };
    std::cout << "Size: " << map.size() << std::endl;

    map.clear();
    std::cout << "Size: " << map.size() << std::endl;

    return 0;
}
```

```
Size: 5
Size: 0
```
