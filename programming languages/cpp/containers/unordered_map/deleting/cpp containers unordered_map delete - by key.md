---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - deleting|<=]]

Метод `erase` по ключу возвращает
- __0__ - не найдено
- __1__ - удалено

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
    std::cout
        << "erase(" + key << ") => "
        << uomap.erase(key) << std::endl;
}
```

```
{one, 1}
{two, 2}
{three, 3}
erase(two) => 1
erase(twotwo) => 0
{one, 1}
{three, 3}
```
