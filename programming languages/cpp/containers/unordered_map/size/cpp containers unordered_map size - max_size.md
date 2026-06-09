---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - size|<=]]

Метод  `max_size` возвращает ёмкость

```cpp
#include <iostream>
#include <string>
#include <unordered_map>

int main() {
    std::unordered_map<std::string, int> map {
        {"one", 1}
    };

    std::cout
        << "max_size: "
        << map.max_size()
        << std::endl;

    return 0;
}
```

```
max_size: 329406144173384850
```
