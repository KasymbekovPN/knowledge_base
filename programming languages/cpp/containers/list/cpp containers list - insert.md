---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - list
---
[[_cpp containers list|<=]]

Метод _insert()_ вставляет элемент в указанную позицию.

```cpp
#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

int main() {
    std::list<int> numbers {1, 2, 3};
    print_list(numbers);

    auto it = numbers.begin();
    std::advance(it, 2);
    numbers.insert(it, 42);
    print_list(numbers);

    return 0;
}

template<typename T>
void print_list(const std::list<T>& list) {
    for (auto &&item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 3 
1 2 42 3
```

---


### 4. **Удаление элементов**

#### a) **Метод `pop_back()`**
Удаляет последний элемент списка.

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> myList = {1, 2, 3, 4};
    myList.pop_back(); // Удаляем последний элемент

    for (int num : myList) {
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
Удаляет первый элемент списка.

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> myList = {1, 2, 3, 4};
    myList.pop_front(); // Удаляем первый элемент

    for (int num : myList) {
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
#include <list>

int main() {
    std::list<int> myList = {1, 2, 3, 4, 5};
    auto it = myList.begin();
    std::advance(it, 2); // Перемещаем итератор на позицию 2
    myList.erase(it); // Удаляем элемент на позиции 2

    for (int num : myList) {
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

#### d) **Метод `remove()`**
Удаляет все элементы с указанным значением.

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> myList = {1, 2, 3, 2, 4};
    myList.remove(2); // Удаляем все элементы со значением 2

    for (int num : myList) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 3 4 
```

---

### 5. **Размер списка**

#### a) **Метод `size()`**
Возвращает количество элементов в списке.

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> myList = {1, 2, 3, 4, 5};
    std::cout << "Size: " << myList.size() << std::endl; // 5
    return 0;
}
```

**Вывод:**
```
Size: 5
```

---

#### b) **Метод `empty()`**
Проверяет, пуст ли список.

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> myList;
    std::cout << "Is empty: " << (myList.empty() ? "Yes" : "No") << std::endl; // Yes
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
Сортирует элементы списка.

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> myList = {5, 3, 1, 4, 2};
    myList.sort(); // Сортировка по возрастанию

    for (int num : myList) {
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
#include <list>

int main() {
    std::list<int> myList = {1, 2, 3, 4, 5};
    myList.reverse(); // Реверс списка

    for (int num : myList) {
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
- `std::list` предоставляет эффективные операции добавления и удаления элементов в любом месте списка.
- Доступ к элементам осуществляется через итераторы.
- Основные методы: `push_back`, `push_front`, `insert`, `pop_back`, `pop_front`, `erase`, `remove`, `sort`, `reverse`.
- `std::list` не поддерживает произвольный доступ по индексу, но идеально подходит для задач, где требуется частое изменение структуры списка.