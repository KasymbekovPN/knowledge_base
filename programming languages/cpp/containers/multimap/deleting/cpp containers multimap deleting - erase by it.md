---
tags:
  - programming-language
  - cpp
  - container
  - multimap
---
[[_cpp containers multimap - deleting|<=]]


```cpp
#include <iostream>
#include <map>

void _print_mmap(const std::multimap<int, std::string>&, std::string);

int main() {
    std::multimap<int, std::string> mmap {
        {1, "one"},
        {2, "two"},
        {2, "two two"},
        {3, "three"},
        {3, "three three"},
        {3, "three three three"}
    };
    _print_mmap(mmap, "BEFORE");

    mmap.erase(mmap.find(2));
    _print_mmap(mmap, "AFTER");

    return 0;
}

void _print_mmap(
	const std::multimap<int, std::string>& mmap,
	std::string label) {
    std::cout
        << "### " << label << " ###"
        << std::endl;
    for (auto &[key, value]: mmap) {
        std::cout
         << "{" << key
         << ", " << value
         << "}" << std::endl;
    }
}
```

```
### BEFORE ###
{1, one}
{2, two}
{2, two two}
{3, three}
{3, three three}
{3, three three three}
### AFTER ###
{1, one}
{2, two two}
{3, three}
{3, three three}
{3, three three three}
```
```
```

----
### Удаление элементов

```cpp

// 3. Удаление диапазона
auto range = mmap.equal_range(3);
mmap.erase(range.first, range.second);


// Очистка контейнера
mmap.clear();

```

### Другие полезные методы

```cpp
// Проверка на пустоту
bool empty = mmap.empty();

// Количество элементов
size_t size = mmap.size();



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