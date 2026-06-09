---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - addition|<=]]

Метод `insert()` добавляет новую пару ключ-значение в `std::map`. Если ключ уже существует, новая пара не добавляется, и операция заканчивается успешно, не перезаписывая существующие данные.

Метод `insert()` возвращает `pair<iterator, bool>`
- _first_ - итератор на элемент
- _second_ - true: добавлен, false: уже существует

Метод `insert_or_assign` появился в C++17 и предоставляет более удобный способ вставки или обновления элементов в `std::map` по сравнению с традиционными методами.
- Вставляет элемент, если ключ не существует
- Обновляет значение, если ключ уже существует
- Возвращает больше информации, чем стандартный `insert`

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
    _print_pair(map.insert(std::make_pair("hello", 1)));
    _print_pair(map.insert(std::make_pair("hello", 1)));

    // C++11
    _print_pair(map.insert({"world", 2}));
    _print_pair(map.insert({"world", 2}));

    std::map<std::string, int> map1 = {{"aaa", 3}, {"bbb", 4}};
    // C++11
    map.insert(map1.begin(), map1.end());
    _print_map(map);

    // C++17
    _print_pair(map.insert_or_assign("ccc", 42));
    _print_pair(map.insert_or_assign("ccc", 45));

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
{hello, 1, true}
{hello, 1, false}
{world, 2, true}
{world, 2, false}
{aaa, 3}
{bbb, 4}
{hello, 1}
{world, 2}
{ccc, 42, true}
{ccc, 45, false}
```

---

The C++ `std::map` container (from the `<map>` header) provides several ways to insert elements. Here are the main methods:

## 1. Using `insert()` with `pair`

cpp

Copy

#include <map>
#include <string>

std::map<int, std::string> myMap;

// Insert using make_pair
myMap.insert(std::make_pair(1, "Apple"));

// Insert using pair constructor
myMap.insert(std::pair<int, std::string>(2, "Banana"));

## 2. Using `insert()` with Initializer List (C++11+)

cpp

Copy

myMap.insert({3, "Cherry"});  // C++11 and later

## 3. Using `emplace()` (C++11+)

More efficient as it constructs the element in-place:

cpp

Copy

myMap.emplace(4, "Date");

## 4. Using `operator[]`

This inserts if the key doesn't exist, or updates if it does:

cpp

Copy

myMap[5] = "Elderberry";  // Inserts new element
myMap[1] = "Apricot";     // Updates existing element with key 1

## Checking Insertion Results

The `insert` methods return a `pair<iterator, bool>` where:

- `first` is an iterator to the element
    
- `second` is a bool indicating whether insertion occurred (true) or the key already existed (false)
    

cpp

Copy

auto result = myMap.insert({2, "Blueberry"});
if (!result.second) {
    std::cout << "Key 2 already exists with value: " << result.first->second << "\n";
}

## Inserting Multiple Elements

cpp

Copy

std::map<int, std::string> otherMap = {{6, "Fig"}, {7, "Grape"}};
myMap.insert(otherMap.begin(), otherMap.end());

## Notes:

- `std::map` keys are unique - inserting with an existing key won't overwrite (except with `operator[]`)
    
- Elements are always sorted by key
    
- For C++17+, you can use `try_emplace()` and `insert_or_assign()` for more control

### Основные методы `std::map`

#### 1. Вставка элементов
```cpp
m.insert({"orange", 7});       // Через insert
m.emplace("pear", 2);          // Через emplace (эффективнее)
m["grape"] = 4;                // Через оператор []
```

1.1. добавление через []

#### 2. Доступ к элементам
```cpp
int val1 = m["apple"];         // Доступ через [] (создает элемент, если нет)
int val2 = m.at("banana");     // Доступ через at (бросает исключение, если нет)
```

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

---

### Пример использования
```cpp
#include <iostream>
#include <map>

int main() {
    std::map<std::string, int> fruits = {
        {"apple", 5},
        {"banana", 3},
        {"cherry", 8}
    };

    // Вставка
    fruits.emplace("orange", 2);
    fruits["kiwi"] = 9;

    // Поиск и изменение
    if (fruits.count("banana")) {
        fruits["banana"] += 2;
    }

    // Удаление
    fruits.erase("cherry");

    // Вывод
    for (const auto& [fruit, count] : fruits) {
        std::cout << fruit << ": " << count << std::endl;
    }

    return 0;
}
```

**Вывод:**
```
apple: 5
banana: 5
kiwi: 9
orange: 2
```

---

### Особенности:
1. **Уникальные ключи** – каждый ключ встречается только один раз.
2. **Автоматическая сортировка** – элементы всегда упорядочены по ключу.
3. **Производительность** – поиск, вставка и удаление за `O(log n)`.
4. **Оператор `[]`** – создает элемент, если ключа нет (со значением по умолчанию).

Для хранения неуникальных ключей используйте `std::multimap`. Для неупорядоченного хранения – `std::unordered_map`.