---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - init|<=]]

Создание `unordered_map` c указанием хеш-функции и функции сравнения

```cpp
#include <iostream>
#include <unordered_map>

template <typename K, typename V, typename H, typename E>
void _print_map(const std::unordered_map<K, V, H, E>&);

int main() {
    auto hash_fn = [](const std::string& key) {
        return std::hash<std::string>()(key);
    };

    auto equal_fn = [](const std::string& lhs, const std::string &rhs){
        return lhs == rhs;
    };

    std::unordered_map<
	    std::string,
	    int,
	    decltype(hash_fn),
	    decltype(equal_fn)> map;
    map.insert({"one", 1});
    map.insert({"two", 2});
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
