---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - deleting|<=]]

Метод `erase` по диапазону удаляет в диапазоне `[first_it, last_it)`

```cpp
#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>&);

int main() {
    std::unordered_map<std::string, int> map {
        {"one", 1},
        {"two", 2},
        {"three", 3},
        {"four", 4},
        {"five", 5}
    };
    _print_map(map);

    auto first_it = map.find("two");
    auto last_it = map.find("five");
    if (first_it != map.end() && last_it != map.end()) {
        map.erase(first_it, last_it);
    }

    _print_map(map);

    return 0;
}

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>& uomap) {
    std::cout << "###" << std::endl;
    for (auto &[key, value]: uomap) {
        std::cout
            << "{" << key
            << ", " << value
            << "}" << std::endl;
    }
}
```

```
###
{one, 1}
{two, 2}
{three, 3}
{five, 5}
{four, 4}
###
{one, 1}
{five, 5}
{four, 4}
```




---

# Удаление элементов из `std::unordered_map` в C++

### 4. Полная очистка (`clear`)
```cpp
umap.clear();  // Удаляет все элементы
```



## Примеры использования

### Удаление с проверкой (C++11 и новее)
```cpp
std::unordered_map<int, std::string> map = {
    {1, "one"},
    {2, "two"},
    {3, "three"}
};

// Удаление с получением следующего итератора
for (auto it = map.begin(); it != map.end(); ) {
    if (it->first % 2 == 0) {  // Удаляем четные ключи
        it = map.erase(it);    // erase возвращает следующий итератор
    } else {
        ++it;
    }
}
```

### Удаление по условию (C++20)
```cpp
std::erase_if(map, [](const auto& item) {
    return item.second.size() > 3;  // Удаляем элементы с длиной строки > 3
});
```


---

# 1

# `std::unordered_map` в C++

### Удаление элементов

```cpp
// По ключу (возвращает количество удаленных элементов)
size_t count = map1.erase("apple");

// По итератору
auto it = map1.find("banana");
if (it != map1.end()) {
    map1.erase(it);
}

// Очистка всего контейнера
map1.clear();
```

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

## Итерация по элементам

```cpp
// Range-based for loop (C++11)
for (const auto& [key, value] : map1) {
    std::cout << key << ": " << value << "\n";
}

// Использование итераторов
for (auto it = map1.begin(); it != map1.end(); ++it) {
    std::cout << it->first << ": " << it->second << "\n";
}
```

## Пользовательские типы в качестве ключа

Для использования пользовательских типов в качестве ключа необходимо:

1. Определить хеш-функцию
2. Определить оператор сравнения (==)

```cpp
struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

// Специализация хеш-функции
namespace std {
    template<> 
    struct hash<Point> {
        size_t operator()(const Point& p) const {
            return hash<int>()(p.x) ^ (hash<int>()(p.y) << 1);
        }
    };
}

// Использование
std::unordered_map<Point, std::string> point_map;
point_map[{1, 2}] = "First point";
```

## Преимущества перед std::map

1. **Быстрее доступ**: O(1) в среднем против O(log n) в map
2. **Эффективнее по памяти**: Обычно требует меньше памяти
3. **Лучше для простых ключей**: Для int, string и т.д.

## Недостатки

1. **Нет порядка элементов**
2. **Худшая производительность в худшем случае**: O(n) вместо O(log n)
3. **Зависимость от качества хеш-функции**

## Пример использования

```cpp
#include <iostream>
#include <unordered_map>
#include <string>

int main() {
    std::unordered_map<std::string, double> price_map = {
        {"milk", 1.20},
        {"bread", 0.80},
        {"cheese", 2.50}
    };

    // Добавление элемента
    price_map["eggs"] = 1.50;

    // Обновление элемента
    price_map["bread"] = 0.90;

    // Поиск элемента
    if (price_map.find("milk") != price_map.end()) {
        std::cout << "Milk price: " << price_map["milk"] << "\n";
    }

    // Итерация по элементам
    for (const auto& [product, price] : price_map) {
        std::cout << product << " costs " << price << " dollars\n";
    }

    return 0;
}
```


---

# 3

c++ unordered_map методы

# Методы `std::unordered_map` в C++

`std::unordered_map` предоставляет множество методов для работы с хеш-таблицей. Рассмотрим основные категории методов:

## 1. Основные операции

### Вставка элементов
```cpp
// operator[] - вставка или изменение значения
std::unordered_map<std::string, int> map;
map["apple"] = 5;  // Вставка
map["apple"] = 10; // Изменение

// insert - вставка пары (не заменяет существующий элемент)
auto result = map.insert({"banana", 3});
if (!result.second) {
    std::cout << "Элемент уже существует\n";
}

// emplace - конструирование элемента на месте
map.emplace("orange", 2);  // Аналогично insert, но более эффективно
```

### Доступ к элементам
```cpp
// operator[] - доступ с созданием элемента при отсутствии
int val = map["apple"];

// at() - доступ с проверкой границ
try {
    int val = map.at("apple");
} catch (const std::out_of_range& e) {
    std::cout << "Ключ не найден\n";
}

// find - поиск без создания элемента
auto it = map.find("banana");
if (it != map.end()) {
    std::cout << "Найдено: " << it->second << "\n";
}
```

### Удаление элементов
```cpp
// erase по ключу (возвращает количество удаленных элементов)
size_t count = map.erase("apple");

// erase по итератору
auto it = map.find("banana");
if (it != map.end()) {
    map.erase(it);
}

// erase диапазон
map.erase(map.begin(), map.end());  // Очистка всей map

// clear - полная очистка
map.clear();
```

## 2. Информационные методы

```cpp
// Проверка на пустоту
bool empty = map.empty();

// Количество элементов
size_t size = map.size();

// Максимально возможный размер
size_t max_size = map.max_size();

// Проверка наличия ключа
if (map.contains("apple")) {  // C++20
    std::cout << "Ключ найден\n";
}

// Количество элементов с ключом (0 или 1)
size_t count = map.count("apple");
```

## 3. Управление хеш-таблицей

```cpp
// Коэффициент загрузки
float lf = map.load_factor();  // Текущий коэффициент
map.max_load_factor(0.75f);    // Установка нового значения

// Резервирование места
map.reserve(100);  // Гарантирует место для ~100 элементов без рехеширования

// Количество bucket'ов
size_t buckets = map.bucket_count();

// Размер конкретного bucket'а
size_t bucket_size = map.bucket_size(0);

// Bucket для конкретного ключа
size_t bucket = map.bucket("apple");
```

## 4. Итераторы

```cpp
// Основные итераторы
auto begin = map.begin();
auto end = map.end();

// Константные итераторы
auto cbegin = map.cbegin();
auto cend = map.cend();

// Итерация по всем элементам
for (const auto& [key, value] : map) {
    std::cout << key << ": " << value << "\n";
}

// Итерация по bucket'ам
for (size_t i = 0; i < map.bucket_count(); ++i) {
    for (auto it = map.begin(i); it != map.end(i); ++it) {
        std::cout << it->first << ": " << it->second << "\n";
    }
}
```

## 5. Специальные методы (C++17+)

```cpp
// try_emplace - вставляет, если ключа нет (без лишних копирований)
auto [it, inserted] = map.try_emplace("apple", 5);
if (inserted) {
    std::cout << "Элемент вставлен\n";
}

// insert_or_assign - вставляет или перезаписывает значение
auto [it, inserted] = map.insert_or_assign("apple", 10);
if (!inserted) {
    std::cout << "Значение перезаписано\n";
}

// extract - извлечение узла без перераспределения памяти
auto node = map.extract("apple");
if (!node.empty()) {
    std::cout << "Извлечено: " << node.key() << ": " << node.mapped() << "\n";
}

// merge - слияние двух unordered_map
std::unordered_map<std::string, int> other = {{"pear", 3}, {"kiwi", 2}};
map.merge(other);  // Элементы other перемещаются в map
```

## 6. Методы для пользовательских типов

При использовании пользовательских типов в качестве ключа:

```cpp
struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

namespace std {
    template<>
    struct hash<Point> {
        size_t operator()(const Point& p) const {
            return hash<int>()(p.x) ^ (hash<int>()(p.y) << 1);
        }
    };
}

std::unordered_map<Point, std::string> point_map;
point_map[{1, 2}] = "Point A";
```

## Пример использования методов

```cpp
#include <iostream>
#include <unordered_map>
#include <string>

int main() {
    std::unordered_map<std::string, int> word_counts;
    
    // Вставка элементов
    word_counts.insert({"hello", 1});
    word_counts.emplace("world", 2);
    word_counts["cpp"] = 3;
    
    // Проверка и изменение значений
    if (word_counts.contains("hello")) {
        word_counts.at("hello") += 1;
    }
    
    // Итерация
    for (const auto& [word, count] : word_counts) {
        std::cout << word << ": " << count << "\n";
    }
    
    // Удаление
    word_counts.erase("cpp");
    
    // Информация о хеш-таблице
    std::cout << "Load factor: " << word_counts.load_factor() << "\n";
    std::cout << "Bucket count: " << word_counts.bucket_count() << "\n";
    
    return 0;
}
```

Эти методы предоставляют полный контроль над `unordered_map`, позволяя эффективно работать с хеш-таблицами в C++.