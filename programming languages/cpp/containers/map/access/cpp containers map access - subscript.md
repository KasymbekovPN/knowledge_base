---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - access|<=]]

Если элемент не существует, то бросается исключение.

```cpp
#include <iostream>
#include <map>
#include <string>

void _print_item(const std::map<std::string, int>&, std::string);

int main() {
    std::map<std::string, int> map = {
        {"one", 1},
        {"two", 2},
    };

    _print_item(map, "one");
    _print_item(map, "two");
    _print_item(map, "three");

    return 0;
}

void _print_item(const std::map<std::string, int>& map, std::string key) {
    try {
        int value = map.at(key);
        std::cout
            << "{key: " << key
            << ", value: " << value
            << "}" << std::endl;
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}
```

```
{key: one, value: 1}
{key: two, value: 2}
invalid map<K, T> key
```


---

#### 3. Поиск элементов
```cpp
auto it = m.find("cherry");    // Возвращает итератор
if (it != m.end()) {
    std::cout << it->second;   // Вывод: 8
}

bool exists = m.count("apple"); // Возвращает 1 или 0
```

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
