---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - size|<=]]

Метод  `empty` проверяет на пустоту

```cpp
#include <iostream>
#include <unordered_map>

void _test_empty(const std::unordered_map<int, int>&);

int main() {
    std::unordered_map<int, int> empty_map;
    _test_empty(empty_map);

    std::unordered_map<int, int> not_empty_map {
        {1, 1}
    };
    _test_empty(not_empty_map);

    return 0;
}

void _test_empty(const std::unordered_map<int, int>& map) {
    std::cout
        << "Is it empty? "
        << std::boolalpha
        << map.empty()
        << std::noboolalpha
        << std::endl;
}
```

```
Is it empty? true
Is it empty? false
```




---
---

# 1

# `std::unordered_map` в C++
## Полезные методы

```cpp
// Проверка на пустоту
bool empty = map1.empty();

// Количество элементов
size_t size = map1.size();

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
