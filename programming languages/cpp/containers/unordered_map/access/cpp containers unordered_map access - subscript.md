---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - access|<=]]

Оператор `[]` (индексация)

- **Плюсы**: Простой синтаксис
- **Минусы**: Создает элемент, если ключа нет (может быть нежелательно)

```cpp
#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>&);

int main() {
    std::unordered_map<std::string, int> uomap {
        {"one", 1}
    };
    std::cout << " << " << uomap["one"] << std::endl;
    std::cout << " << " << uomap["two"] << std::endl;

    _print_map(uomap);

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
 << 1
 << 0
{ one, 1 }
{ two, 0 }
```
