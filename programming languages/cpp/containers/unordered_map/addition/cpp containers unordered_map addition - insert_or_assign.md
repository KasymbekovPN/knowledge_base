---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - addition|<=]]

Метод _insert_or_assign_ доступен с __C++17__

- Вставляет, если ключа нет
- Обновляет значение, если ключ есть
- Возвращает `pair<iterator, bool>`
	- iterator на вставленный/существующий элемент
	- bool: true если вставка, false если ключ уже был

```cpp
#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _print_pair(
	const std::pair<
		typename std::unordered_map<K, V>::iterator, bool>&);

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>&);

int main() {
    std::unordered_map<std::string, int> map;
    _print_pair<std::string, int>(map.insert_or_assign("one", 1));
    _print_pair<std::string, int>(map.insert_or_assign("one", 11));
    _print_pair<std::string, int>(map.insert_or_assign("one", 11));
    _print_pair<std::string, int>(map.insert_or_assign("two", 2));

    _print_map(map);

    return 0;
}

template <typename K, typename V>
void _print_pair(
	const std::pair<
		typename std::unordered_map<K, V>::iterator, bool>& pair) {
    std::cout
     << "[ (" << pair.first->first
     << ", " << pair.first->second
     << " ), " << pair.second
     << " ]" << std::endl;
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
[ (one, 1 ), 1 ]
[ (one, 11 ), 0 ]
[ (one, 11 ), 0 ]
[ (two, 2 ), 1 ]
{ one, 11 }
{ two, 2 }
```
