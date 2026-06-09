---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - forward_list
---
[[_cpp containers forward_list - init|<=]]

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist;
    std::cout
	    << "Is empty? "
	    << (flist.empty() ? "Yes" : "No") << std::endl;

    return 0;
}
```

```
Is empty? Yes
```

---
#### b) Список с начальными значениями
```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist = {1, 2, 3, 4, 5};
    std::cout << "Elements: ";
    for (int num : flist) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
Elements: 1 2 3 4 5 
```

---

### 2. **Доступ к элементам**

#### a) **Метод `front()`**
Возвращает первый элемент списка.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist = {1, 2, 3, 4, 5};
    std::cout << "First element: " << flist.front() << std::endl; // 1
    return 0;
}
```

**Вывод:**
```
First element: 1
```

---

#### b) **Итерация по элементам**
Для доступа к элементам используются итераторы.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist = {1, 2, 3, 4, 5};

    // Использование range-based for
    std::cout << "Range-based for: ";
    for (int num : flist) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    // Использование итераторов
    std::cout << "Using iterators: ";
    for (auto it = flist.begin(); it != flist.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

**Вывод:**
```
Range-based for: 1 2 3 4 5 
Using iterators: 1 2 3 4 5 
```

---

### 3. **Добавление элементов**

#### a) **Метод `push_front()`**
Добавляет элемент в начало списка.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist = {2, 3, 4};
    flist.push_front(1); // Добавляем 1 в начало

    for (int num : flist) {
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

#### b) **Метод `insert_after()`**
Вставляет элемент после указанной позиции.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist = {1, 2, 4};
    auto it = flist.begin();
    std::advance(it, 1); // Перемещаем итератор на второй элемент
    flist.insert_after(it, 3); // Вставляем 3 после второго элемента

    for (int num : flist) {
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

#### a) **Метод `pop_front()`**
Удаляет первый элемент списка.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist = {1, 2, 3, 4};
    flist.pop_front(); // Удаляем первый элемент

    for (int num : flist) {
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

#### b) **Метод `erase_after()`**
Удаляет элемент после указанной позиции.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist = {1, 2, 3, 4};
    auto it = flist.begin();
    std::advance(it, 1); // Перемещаем итератор на второй элемент
    flist.erase_after(it); // Удаляем элемент после второго (третий элемент)

    for (int num : flist) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 4 
```

---

#### c) **Удаление диапазона элементов**
Метод `erase_after` может удалить диапазон элементов.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist = {1, 2, 3, 4, 5};
    auto it1 = flist.begin();
    auto it2 = flist.begin();
    std::advance(it1, 1); // it1 указывает на второй элемент
    std::advance(it2, 3); // it2 указывает на четвертый элемент
    flist.erase_after(it1, it2); // Удаляем элементы после второго до четвертого

    for (int num : flist) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 5 
```

---

### 5. **Размер списка**

#### a) **Метод `empty()`**
Проверяет, пуст ли список.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist;
    std::cout << "Is empty: " << (flist.empty() ? "Yes" : "No") << std::endl; // Yes
    return 0;
}
```

**Вывод:**
```
Is empty: Yes
```

---

#### b) **Метод `max_size()`**
Возвращает максимально возможный размер списка.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist;
    std::cout << "Max size: " << flist.max_size() << std::endl;
    return 0;
}
```

**Вывод:**
```
Max size: <очень большое число>
```

---

### 6. **Сортировка и реверс**

#### a) **Метод `sort()`**
Сортирует элементы списка.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist = {5, 3, 1, 4, 2};
    flist.sort(); // Сортировка по возрастанию

    for (int num : flist) {
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
Переворачивает порядок элементов в списке.

```cpp
#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist = {1, 2, 3, 4, 5};
    flist.reverse(); // Реверс списка

    for (int num : flist) {
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
- `std::forward_list` — это односвязный список, который поддерживает только однонаправленное движение.
- Основные методы: `push_front`, `insert_after`, `pop_front`, `erase_after`, `sort`, `reverse`.
- Итерация по элементам возможна только от начала к концу.
- `std::forward_list` более компактен по памяти, чем `std::list`, но менее гибок.