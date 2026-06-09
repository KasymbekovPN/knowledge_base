---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - size|<=]]

Метод  `count` возвращает количество элементов по ключу

```cpp
#include <iostream>
#include <string>
#include <unordered_map>

template <typename K, typename V>
void _test_count(const std::unordered_map<K, V>&, K);

int main() {
    std::unordered_map<std::string, int> map {
        {"one", 1}
    };

    _test_count(map, std::string{"one"});
    _test_count(map, std::string{"two"});

    return 0;
}

template <typename K, typename V>
void _test_count(const std::unordered_map<K, V>& map, K key) {
    std::cout
        << "Count: "
        << map.count(key)
        << std::endl;
}
```

```
Count: 1
Count: 0
```
