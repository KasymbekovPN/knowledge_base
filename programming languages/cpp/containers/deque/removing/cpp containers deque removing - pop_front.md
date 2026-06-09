---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - removing|<=]]

Метод _pop_front()_ удаляет первый элемент.

В C++ вызов метода `pop_front()` на пустом `std::deque` приводит к **неопределенному поведению** (undefined behavior)

```cpp
#include <iostream>
#include <string>
#include <deque>

void _print_deque(const std::deque<int>&);
void _pop_front(std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 2};
    _print_deque(deq);

    _pop_front(deq);
    _print_deque(deq);

    _pop_front(deq);
    _print_deque(deq);

    _pop_front(deq);
    _print_deque(deq);

    return 0;
}

void _print_deque(const std::deque<int>& deque) {
    std::cout << "{";
    for (auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << "}" << std::endl;
}

void _pop_front(std::deque<int>& deque) {
    if (!deque.empty()) {
        deque.pop_front();
    } else {
        std::cout << "Empty!" << std::endl;
    }
}
```

```
{1 2 }
{2 }
{}
Empty!
{}
```


---
---
---

#### c) **Метод `erase()`**
Удаляет элемент или диапазон элементов по итератору.

```cpp
#include <iostream>
#include <deque>

int main() {
    std::deque<int> dq = {1, 2, 3, 4, 5};
    auto it = dq.begin();
    std::advance(it, 2); // Перемещаем итератор на третий элемент
    dq.erase(it); // Удаляем третий элемент

    for (int num : dq) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 4 5 
```

---

### 5. **Размер очереди**

#### a) **Метод `size()`**
Возвращает количество элементов в очереди.

```cpp
#include <iostream>
#include <deque>

int main() {
    std::deque<int> dq = {1, 2, 3, 4, 5};
    std::cout << "Size: " << dq.size() << std::endl; // 5
    return 0;
}
```

**Вывод:**
```
Size: 5
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