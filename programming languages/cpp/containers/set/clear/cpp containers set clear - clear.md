---
tags:
  - programming-language
  - cpp
  - container
  - set
---
[[_cpp containers set - clear|<=]]

Метод `clear()` удаляет все элементы из std::set.

```cpp
#include <iostream>
#include <set>

int main() {
    std::set<int> s {1, 2, 3};
    std::cout << "size: " << s.size() << std::endl;

    s.clear();
    std::cout << "size: " << s.size() << std::endl;

    return 0;
}
```

```
size: 3
size: 0
```

---

10. **`erase(const T& value)`**:
   - Удаляет элемент с указанным значением.
   - Возвращает количество удаленных элементов (0 или 1, так как элементы уникальны).
   - Пример:
     ```cpp
     std::set<int> s = {1, 2, 3};
     s.erase(2); // Удаляет элемент 2
     ```



---

### Пример использования всех методов

```cpp
#include <iostream>
#include <set>

int main() {
    std::set<int> s = {3, 1, 4, 1, 5}; // Дубликаты игнорируются

    // Вставка элемента
    s.insert(2);

    // Поиск элемента
    auto it = s.find(3);
    if (it != s.end()) {
        std::cout << "Element 3 found." << std::endl;
    }

    // Удаление элемента
    s.erase(1);

    // Вывод элементов
    for (int num : s) {
        std::cout << num << " "; // 2 3 4 5
    }

    // Проверка на пустоту
    if (!s.empty()) {
        std::cout << "\nSet is not empty." << std::endl;
    }

    // Очистка set
    s.clear();
    std::cout << "Size after clear: " << s.size() << std::endl; // 0

    return 0;
}
```

**Вывод:**
```
Element 3 found.
2 3 4 5 
Set is not empty.
Size after clear: 0
```

---

### Итог

- `std::set` хранит уникальные элементы в отсортированном порядке.
- Основные методы: `insert`, `erase`, `find`, `count`, `size`, `empty`, `clear`, `begin`, `end`, `lower_bound`, `upper_bound`, `equal_range`.
- Используйте `std::set`, когда нужен контейнер с уникальными элементами и быстрым поиском.