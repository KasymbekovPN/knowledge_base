---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - list
---
[[_cpp containers list|<=]]

```cpp
#include <iostream>
#include <list>

int main(int argc, char const *argv[]) {
    std::list<int> numbers {1, 2, 3};

    for (auto &&number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;

    for (auto &number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;

    for (const auto &number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;

    for (auto it {numbers.begin()}; it != numbers.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

```
1 2 3 
1 2 3
1 2 3
1 2 3
```


---

### 3. **Добавление элементов**

#### a) **Метод `push_back()`**
Добавляет элемент в конец списка.

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> myList = {1, 2, 3};
    myList.push_back(4); // Добавляем 4 в конец

    for (int num : myList) {
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
Добавляет элемент в начало списка.

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> myList = {2, 3, 4};
    myList.push_front(1); // Добавляем 1 в начало

    for (int num : myList) {
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
#include <list>

int main() {
    std::list<int> myList = {1, 2, 4};
    auto it = myList.begin();
    std::advance(it, 2); // Перемещаем итератор на позицию 2
    myList.insert(it, 3); // Вставляем 3 перед позицией 2

    for (int num : myList) {
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