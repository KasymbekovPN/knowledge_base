---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - deleting|<=]]

Метод `erase` по итератору возвращает недействительный итератор

```cpp
#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>&);

template <typename K, typename V>
void _test_erase(std::unordered_map<K, V>&, const std::string&);

int main() {
    std::unordered_map<std::string, int> map {
        {"one", 1},
        {"two", 2},
        {"three", 3}
    };
    _print_map(map);

    _test_erase(map, "two");
    _test_erase(map, "twotwo");
    _print_map(map);

    return 0;
}

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>& uomap) {
    for (auto &[key, value]: uomap) {
        std::cout
            << "{" << key
            << ", " << value
            << "}" << std::endl;
    }
}

template <typename K, typename V>
void _test_erase(std::unordered_map<K, V>& uomap, const std::string& key) {
    if (auto it = uomap.find(key); it != uomap.end()) {
        std::cout
            << "erase(" + key << ") => "
            << uomap.erase(it)->second << std::endl;
    } else {
        std::cout << "Not found" << std::endl;
    }
}
```

```
{one, 1}
{two, 2}
{three, 3}
erase(two) => 3
Not found
{one, 1}
{three, 3}
```
