---
tags:
  - programming-language
  - cpp
  - container
  - multimap
---
[[_cpp containers multimap - find|<=]]

Метод `find` возвращает итератор к первому значению найденному по ключу

```cpp
#include <iostream>
#include <map>

void _test_find(const std::multimap<int, std::string>&, int);

int main() {
    std::multimap<int, std::string> mmap {
        {1, "one"},
        {1, "one one"},
        {2, "two"},
        {2, "two two"},
        {3, "three"},
        {3, "three three"}
    };

    _test_find(mmap, 1);
    _test_find(mmap, 2);
    _test_find(mmap, 3);
    _test_find(mmap, 42);

    return 0;
}

void _test_find(const std::multimap<int, std::string>& mmap, int key) {
    auto it = mmap.find(key);
    if (it != mmap.end()) {
        std::cout
            << "{key: " << it->first
            << ", value: "  << it->second
            << "}" << std::endl;
    } else {
        std::cout << "Not found" << std::endl;
    }
}
```

```
{key: 1, value: one}
{key: 2, value: two}
{key: 3, value: three}
Not found
```

----
### Поиск элементов

```cpp

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