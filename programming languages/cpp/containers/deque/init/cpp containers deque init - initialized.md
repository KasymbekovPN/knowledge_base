---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - init|<=]]

```cpp
#include <iostream>
#include <deque>

template <typename T>
void print_deque(const std::deque<T>&);

int main() {
    std::deque<int> deq {1, 2, 3};
    print_deque(deq);

    return 0;
}

template <typename T>
void print_deque(const std::deque<T>& deque) {
    for (auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 3
```

---
### 2. **Доступ к элементам**

#### a) **Методы `front()` и `back()`**
- `front()` возвращает первый элемент.
- `back()` возвращает последний элемент.

```cpp
#include <iostream>
#include <deque>

int main() {
    std::deque<int> dq = {1, 2, 3, 4, 5};
    std::cout << "First element: " << dq.front() << std::endl; // 1
    std::cout << "Last element: " << dq.back() << std::endl;  // 5
    return 0;
}
```

**Вывод:**
```
First element: 1
Last element: 5
```

---

#### b) **Доступ по индексу**
Элементы `std::deque` можно получить по индексу с помощью оператора `[]` или метода `at()`.

```cpp
#include <iostream>
#include <deque>

int main() {
    std::deque<int> dq = {1, 2, 3, 4, 5};

    // Использование оператора []
    std::cout << "Element at index 2: " << dq[2] << std::endl; // 3

    // Использование метода at()
    std::cout << "Element at index 3: " << dq.at(3) << std::endl; // 4

    return 0;
}
```

**Вывод:**
```
Element at index 2: 3
Element at index 3: 4
```

---

### 3. **Добавление элементов**

#### a) **Метод `push_back()`**
Добавляет элемент в конец очереди.

```cpp
#include <iostream>
#include <deque>

int main() {
    std::deque<int> dq = {1, 2, 3};
    dq.push_back(4); // Добавляем 4 в конец

    for (int num : dq) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 3 4 
```

---

#### b) **Метод `push_front()`**
Добавляет элемент в начало очереди.

```cpp
#include <iostream>
#include <deque>

int main() {
    std::deque<int> dq = {2, 3, 4};
    dq.push_front(1); // Добавляем 1 в начало

    for (int num : dq) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 3 4 
```

---

#### c) **Метод `insert()`**
Вставляет элемент в указанную позицию.

```cpp
#include <iostream>
#include <deque>

int main() {
    std::deque<int> dq = {1, 2, 4};
    auto it = dq.begin();
    std::advance(it, 2); // Перемещаем итератор на третий элемент
    dq.insert(it, 3); // Вставляем 3 перед третьим элементом

    for (int num : dq) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 3 4 
```

---

### 4. **Удаление элементов**

#### a) **Метод `pop_back()`**
Удаляет последний элемент.

```cpp
#include <iostream>
#include <deque>

int main() {
    std::deque<int> dq = {1, 2, 3, 4};
    dq.pop_back(); // Удаляем последний элемент

    for (int num : dq) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 3 
```

---

#### b) **Метод `pop_front()`**
Удаляет первый элемент.

```cpp
#include <iostream>
#include <deque>

int main() {
    std::deque<int> dq = {1, 2, 3, 4};
    dq.pop_front(); // Удаляем первый элемент

    for (int num : dq) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
2 3 4 
```

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