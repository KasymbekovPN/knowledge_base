---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - size|<=]]

Метод  `size` возвращает количество элементов

```cpp
#include <iostream>
#include <unordered_map>

void _test_size(const std::unordered_map<int, int>&);

int main() {
    std::unordered_map<int, int> empty_map;
    _test_size(empty_map);

    std::unordered_map<int, int> not_empty_map {
        {1, 1}
    };
    _test_size(not_empty_map);

    return 0;
}

void _test_size(const std::unordered_map<int, int>& map) {
    std::cout
        << "Size: "
        << map.size()
        << std::endl;
}
```

```
Size: 0
Size: 1
```




---
---

# 1

# `std::unordered_map` в C++
## Полезные методы

```cpp

// Количество элементов с определенным ключом (0 или 1)
size_t count = map1.count("apple");

// Максимальный размер
size_t max_size = map1.max_size();
```

## Настройка хеш-таблицы

```cpp
// Установка/получение коэффициента загрузки
map1.max_load_factor(0.75f);  // Установка
float lf = map1.load_factor(); // Получение

// Резервирование места
map1.reserve(100);  // Резервирует место для ~100 элементов
```
