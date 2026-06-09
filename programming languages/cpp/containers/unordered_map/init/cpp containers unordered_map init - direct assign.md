---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - init|<=]]

Прямое присваивание __C++11__

```cpp
#include <iostream>
#include <unordered_map>

template <typename K, typename V, typename H, typename E>
void _print_map(const std::unordered_map<K, V, H, E>&);

int main() {
    std::unordered_map<std::string, int> map {
        {"one", 1},
        {"two", 2}
    };
    _print_map(map);

    return 0;
}

template <typename K, typename V, typename H, typename E>
void _print_map(const std::unordered_map<K, V, H, E>& uomap) {
    for (auto &[key, value]: uomap) {
        std::cout
            << "{ " << key
            << ", " << value
            << " }" << std::endl;
    }
}
```

```
{ one, 1 }
{ two, 2 }
```
