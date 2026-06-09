---
tags:
  - programming-language
  - cpp
  - container
  - multimap
---
[[_cpp containers multimap - addition|<=]]

```cpp
#include <iostream>
#include <map>
#include <vector>

void _print_mmap(const std::multimap<int, std::string>&);

int main() {
    std::vector<std::pair<int, std::string>> source {
        {1, "one"},
        {3, "three"},
        {5, "five"}
    };
    std::multimap<int, std::string> mmap;
    mmap.insert(source.begin(), source.end());

    _print_mmap(mmap);

    return 0;
}

void _print_mmap(const std::multimap<int, std::string>& mmap) {
    for (auto &[key, value]: mmap) {
        std::cout
            << "{key: " << key
            << ", value: " << value
            << "}" << std::endl;
    }
}
```

```
{key: 1, value: one}
{key: 3, value: three}
{key: 5, value: five}
```

---
# Методы `emplace` в std::multimap (C++)

Методы `emplace` предоставляют эффективный способ добавления элементов в `std::multimap` путем конструирования их на месте (in-place construction), что позволяет избежать лишних копирований и перемещений.

## Основные методы emplace

### 1. `emplace` - базовая версия
```cpp
template <class... Args>
iterator emplace(Args&&... args);
```
Конструирует и вставляет элемент из переданных аргументов.

Пример:
```cpp
std::multimap<int, std::string> mmap;

// Эквивалент mmap.insert(std::make_pair(1, "Apple"))
auto it = mmap.emplace(1, "Apple");

// Можно передавать аргументы для конструирования
mmap.emplace(2, 3, 'a');  // std::string(3, 'a') → "aaa"
```

### 2. `emplace_hint` - с подсказкой положения
```cpp
template <class... Args>
iterator emplace_hint(const_iterator hint, Args&&... args);
```
Конструирует элемент на месте с подсказкой о возможной позиции вставки.

Пример:
```cpp
auto hint = mmap.find(1);
if (hint != mmap.end()) {
    mmap.emplace_hint(hint, 3, "Banana");
}
```

## Преимущества emplace перед insert

1. **Избегает создания временных объектов**:
   ```cpp
   // С insert создается временный pair
   mmap.insert(std::make_pair(4, "Cherry"));
   
   // С emplace pair конструируется на месте
   mmap.emplace(4, "Cherry");
   ```

2. **Эффективность для сложных типов**:
   ```cpp
   struct Product {
       Product(int id, std::string name) : id(id), name(std::move(name)) {}
       int id;
       std::string name;
   };
   
   std::multimap<int, Product> products;
   
   // Более эффективно с emplace
   products.emplace(1, 101, "Laptop");  // Конструирует Product(101, "Laptop") на месте
   ```

## Возвращаемое значение

Оба метода возвращают итератор на вставленный элемент.

## Особенности использования

1. **Порядок аргументов**:
   ```cpp
   // Первый аргумент - ключ, остальные - аргументы конструктора значения
   mmap.emplace(key, arg1, arg2, ...);
   ```

2. **Для пользовательских типов**:
   ```cpp
   struct Value {
       Value(int a, double b) {}
   };
   
   std::multimap<int, Value> custom_map;
   custom_map.emplace(1, 42, 3.14);  // Конструирует Value(42, 3.14)
   ```

3. **Совместимость с move-семантикой**:
   ```cpp
   std::string large_data = "Very long string...";
   mmap.emplace(5, std::move(large_data));  // Эффективное перемещение
   ```

## Пример комплексного использования

```cpp
#include <iostream>
#include <map>
#include <string>

int main() {
    std::multimap<std::string, std::pair<int, double>> inventory;
    
    // Вставка с emplace
    inventory.emplace("Laptop", std::make_pair(5, 999.99));
    
    // Более эффективная альтернатива
    inventory.emplace("Phone", 3, 699.99);  // Конструирует pair<int,double>(3, 699.99)
    
    // Использование emplace_hint
    auto hint = inventory.find("Phone");
    if (hint != inventory.end()) {
        inventory.emplace_hint(hint, "Tablet", 2, 399.99);
    }
    
    // Вывод содержимого
    for (const auto& [name, data] : inventory) {
        std::cout << name << ": " << data.first 
                  << " items, $" << data.second << " each\n";
    }
    
    return 0;
}
```

## Рекомендации

1. **Всегда используйте `emplace`** вместо `insert` при работе с сложными или дорогими для копирования типами
2. **Используйте `emplace_hint`**, когда знаете примерное положение для вставки
3. **Для простых типов** разница между `emplace` и `insert` может быть незначительной
4. **В C++17+** для `std::map` также доступны `try_emplace` и `insert_or_assign`

----
### Поиск элементов

```cpp
// 1. find - возвращает итератор к первому найденному элементу
auto it = mmap.find(1);
if (it != mmap.end()) {
    std::cout << "Found: " << it->second << std::endl;
}

// 2. equal_range - возвращает пару итераторов (начало и конец диапазона)
auto range = mmap.equal_range(1);
for (auto i = range.first; i != range.second; ++i) {
    std::cout << i->second << std::endl;
}

// 3. count - количество элементов с заданным ключом
size_t cnt = mmap.count(1);
```

### Удаление элементов

```cpp
// 1. Удаление по ключу (удаляет все элементы с этим ключом)
mmap.erase(1);

// 2. Удаление по итератору
auto it = mmap.find(2);
if (it != mmap.end()) {
    mmap.erase(it);
}

// 3. Удаление диапазона
auto range = mmap.equal_range(3);
mmap.erase(range.first, range.second);
```

### Другие полезные методы

```cpp
// Проверка на пустоту
bool empty = mmap.empty();

// Количество элементов
size_t size = mmap.size();

// Очистка контейнера
mmap.clear();

// Верхняя и нижняя границы
auto lb = mmap.lower_bound(2); // Первый элемент не меньше ключа
auto ub = mmap.upper_bound(4); // Первый элемент больше ключа
```

## Особенности multimap

1. Элементы автоматически сортируются по ключу
2. Поддерживает несколько значений для одного ключа
3. Не предоставляет оператор [] для доступа к элементам
4. Итераторы двунаправленные (можно перемещаться в обоих направлениях)

## Пример использования

```cpp
#include <iostream>
#include <map>
#include <string>

int main() {
    std::multimap<std::string, int> grades = {
        {"Alice", 90},
        {"Bob", 85},
        {"Alice", 95},
        {"Charlie", 88}
    };

    // Добавление еще одной оценки Alice
    grades.emplace("Alice", 92);

    // Вывод всех оценок Alice
    auto range = grades.equal_range("Alice");
    std::cout << "Alice's grades: ";
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << it->second << " ";
    }
    std::cout << std::endl;

    // Удаление всех оценок Bob
    grades.erase("Bob");

    // Вывод всех элементов
    for (const auto& [name, grade] : grades) {
        std::cout << name << ": " << grade << std::endl;
    }

    return 0;
}
```