---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - addition|<=]]

Метод `emplace()` позволяет эффективно вставлять новые элементы, конструируя их на месте. Вместо создания временной пары и последующего копирования/перемещения, элементы создаются непосредственно в структуре данных.

Введен в C++17, метод `try_emplace()` пытается вставить элемент, создавая его на месте. Отличается от `emplace()` тем, что не обновляет существующий элемент, если ключ уже существует.

```cpp
#include <iostream>
#include <map>
#include <string>

void _print_pair(
	const std::pair<std::map<std::string, int>::iterator, bool>&
);
void _print_map(const std::map<std::string, int>&);

int main() {
    std::map<std::string, int> map;
    _print_pair(map.emplace("one", 1));
    _print_pair(map.emplace("one", 11));

    // C++17
    _print_pair(map.try_emplace("two", 2));
    _print_pair(map.try_emplace("two", 21));

    _print_map(map);

    return 0;
}

void _print_map(const std::map<std::string, int>& m) {
    for (auto &pair: m) {
        std::cout << "{" << pair.first
            << ", " << pair.second
            << "}" << std::endl;
    }
}

void _print_pair(
	const std::pair<std::map<std::string, int>::iterator, bool>& pair) {
    std::cout
        << "{" << (*pair.first).first
        << ", " << (*pair.first).second
        << std::boolalpha
        << ", " << pair.second
        << std::noboolalpha
        << "}" << std::endl;
}
```

```
{one, 1, true}
{one, 1, false}
{two, 2, true}
{two, 2, false}
{one, 1}
{two, 2}
```