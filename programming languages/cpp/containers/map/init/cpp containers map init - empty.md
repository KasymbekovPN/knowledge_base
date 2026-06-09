---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - init|<=]]

```cpp
#include <iostream>
#include <map>

int main() {
    std::map<std::string, int> emptym;
    std::cout
        << "Is it empty? "
        << std::boolalpha
        << emptym.empty()
        << std::noboolalpha
        << std::endl;
  
    return 0;
}
```

```
Is it empty? true
```

---

#### 2. Инициализация списком пар
```cpp
std::map<std::string, int> m = {
    {"apple", 5},
    {"banana", 3},
    {"cherry", 8}
};
```

#### 3. Копирование из другого map
```cpp
std::map<std::string, int> m2(m);
```

#### 4. С пользовательским компаратором (сортировка по убыванию)
```cpp
std::map<std::string, int, std::greater<>> m = {
    {"apple", 5},
    {"banana", 3},
    {"cherry", 8}
};
```

---

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