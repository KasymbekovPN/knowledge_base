---
tags:
  - programming-language
  - cpp
  - container
  - multimap
---
[[_cpp containers multimap - find|<=]]

Метод `upper_bound` возвращает первый элемент больше ключа.

```cpp
#include <iostream>
#include <map>

int main() {
    std::multimap<int, std::string> mmap {
        {1, "one"},
        {1, "one one"},
        {2, "two"},
        {2, "two two"},
        {3, "three"},
        {3, "three three"}
    };

    auto it = mmap.upper_bound(2);
    std::cout
        << "{" << it->first
        << ", " << it->second
        << "}" << std::endl;

    return 0;
}
```

```
{3, three}
```
```
```

----

### Другие полезные методы

```cpp


// Проверка на пустоту
bool empty = mmap.empty();

// Количество элементов
size_t size = mmap.size();




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