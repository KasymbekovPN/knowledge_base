---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - deleting|<=]]

```cpp
#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>&);

int main() {
    std::unordered_map<std::string, int> map {
        {"a0", 1},
        {"b0", 2},
        {"c0", 3},
        {"a1", 4},
        {"b1", 5}
    };
    _print_map(map);

    // C++11
    for(auto it = map.begin(); it != map.end(); ) {
        if (it->first[0] == 'a') {
            it = map.erase(it);
        } else {
            ++it;
        }
    }

    // C++20
    std::erase_if(map, [](const auto& item) {
        return item.first[0] == 'b';
    });

    _print_map(map);
  
    return 0;
}

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>& uomap) {
    std::cout << "###" << std::endl;
    for (auto &[key, value]: uomap) {
        std::cout
            << "{" << key
            << ", " << value
            << "}" << std::endl;
    }
}
```

```
###
{b1, 5}
{a0, 1}
{a1, 4}
{b0, 2}
{c0, 3}
###
{c0, 3}
```
