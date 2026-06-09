---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - size|<=]]

Метод _size()_ возвращает количество элементов в очереди.

```cpp
#include <iostream>
#include <deque>

void _print_size(const std::deque<int>&);

int main() {
    std::deque<int> empty_deq {};
    std::deque<int> deq {1, 2, 3};

    _print_size(empty_deq);
    _print_size(deq);

    return 0;
}

void _print_size(const std::deque<int>& deque) {
    std::cout
        << "Deque size is "
        << deque.size() << std::endl;
}
```

```
Deque size is 0
Deque size is 3
```

---

#### b) **Метод `empty()`**
Проверяет, пуста ли очередь.

```cpp
#include <iostream>
#include <deque>

int main() {
    std::deque<int> dq;
    std::cout << "Is empty: " << (dq.empty() ? "Yes" : "No") << std::endl; // Yes
    return 0;
}
```

**Вывод:**
```
Is empty: Yes
```

---

### 6. **Сортировка и реверс**

#### a) **Метод `sort()`**
Сортирует элементы очереди.

```cpp
#include <iostream>
#include <deque>
#include <algorithm> // для std::sort

int main() {
    std::deque<int> dq = {5, 3, 1, 4, 2};
    std::sort(dq.begin(), dq.end()); // Сортировка по возрастанию

    for (int num : dq) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 3 4 5 
```

---

#### b) **Метод `reverse()`**
Переворачивает порядок элементов в очереди.

```cpp
#include <iostream>
#include <deque>
#include <algorithm> // для std::reverse

int main() {
    std::deque<int> dq = {1, 2, 3, 4, 5};
    std::reverse(dq.begin(), dq.end()); // Реверс очереди

    for (int num : dq) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
5 4 3 2 1 
```

---

### Итог
- `std::deque` поддерживает эффективное добавление и удаление элементов как в начале, так и в конце.
- Доступ к элементам возможен по индексу с помощью `[]` или `at()`.
- Основные методы: `push_back`, `push_front`, `pop_back`, `pop_front`, `insert`, `erase`, `sort`, `reverse`.
- `std::deque` — это гибкий контейнер, который сочетает в себе возможности `std::vector` и `std::list`.