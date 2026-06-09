---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - find|<=]]

Метод `count()` возвращает количество элементов по ключу.
- __1__ - если существует 
- __0__ - если нет

```cpp
#include <iostream>
#include <map>

void _test_count(const std::map<std::string, int>&, std::string);

int main() {
    std::map<std::string, int> map = {
        {"one", 1},
        {"two", 2},
    };
    _test_count(map, "one");
    _test_count(map, "two");
    _test_count(map, "three");

    return 0;
}

void _test_count(const std::map<std::string, int>& map, std::string key) {
    std::cout
        << "{key: " << key
        << ", count: " << map.count(key)
        << "}" << std::endl;
}
```

```
{key: one, count: 1}
{key: two, count: 1}
{key: three, count: 0}
```

---

#### 4. Удаление элементов
```cpp
m.erase("banana");             // По ключу
m.erase(m.find("apple"));      // По итератору
m.erase(m.begin(), m.end());   // Диапазон
```

#### 5. Размер и проверка на пустоту
```cpp
std::cout << m.size();         // Количество элементов
std::cout << m.empty();        // Проверка на пустоту
```

#### 6. Обход элементов
```cpp
for (const auto& [key, value] : m) {
    std::cout << key << ": " << value << std::endl;
}
```

#### 7. Границы диапазонов
```cpp
auto lb = m.lower_bound("b");  // Первый элемент >= "b"
auto ub = m.upper_bound("c");  // Первый элемент > "c"
```
