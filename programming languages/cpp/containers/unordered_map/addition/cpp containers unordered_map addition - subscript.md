---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - addition|<=]]

Оператор _[]_
- Если ключа нет - создает новый элемент
- Если ключ есть - обновляет значение
- Всегда возвращает ссылку на значение

```cpp
#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>&);

int main() {
    std::unordered_map<std::string, int> map {};
    map["one"] = 11;
    map["one"] = 1;
    map["two"] = 2;
  
    _print_map(map);

    return 0;
}

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>& uomap) {
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
