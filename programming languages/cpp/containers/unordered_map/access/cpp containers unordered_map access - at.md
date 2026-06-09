---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - access|<=]]

Метод `at()`

- **Плюсы**: Безопасный доступ с проверкой
- **Минусы**: Нужно обрабатывать исключение

```cpp
#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _test_at(const std::unordered_map<K, V>&, const std::string&);


int main() {
    std::unordered_map<std::string, int> uomap {
        {"one", 1}
    };

    _test_at(uomap, "one");
    _test_at(uomap, "two");

    return 0;
}

template <typename K, typename V>
void _test_at(
	const std::unordered_map<K, V>& uomap, const std::string& key) {
    try {
        std::cout << "<= " << uomap.at(key) << std::endl;
    } catch(const std::out_of_range& e) {
        std::cerr << e.what() << std::endl;
    }
}
```

```
<= 1
<= invalid unordered_map<K, T> key
```
