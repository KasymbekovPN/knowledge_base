---
tags:
  - programming-language
  - cpp
  - container
  - queue
---
[[_cpp containers queue - init|<=]]

```cpp
#include <iostream>
#include <queue>

int main() {
    std::queue<int> q;
    std::cout
        << "Queue is empty? "
        << std::boolalpha
        << q.empty()
        << std::noboolalpha
        << std::endl;

    return 0;
}
```

```
Queue is empty? true
```


---
#### 2. **Инициализация с использованием другого контейнера**
Можно использовать другой контейнер, например `std::list` или `std::vector`, для хранения элементов.

```cpp
#include <iostream>
#include <queue>
#include <list>

int main() {
    std::list<int> lst = {1, 2, 3};
    std::queue<int, std::list<int>> q(lst);

    std::cout << "Front element: " << q.front() << std::endl; // 1
    return 0;
}
```

---

### Основные методы `std::queue`

1. **`push(const T& value)`**:
   - Добавляет элемент в конец очереди.
   - Пример:
     ```cpp
     std::queue<int> q;
     q.push(10);
     q.push(20);
     q.push(30);
     ```

2. **`pop()`**:
   - Удаляет элемент из начала очереди.
   - **Важно**: Не вызывайте `pop()` на пустой очереди, это приведет к неопределенному поведению.
   - Пример:
     ```cpp
     std::queue<int> q;
     q.push(10);
     q.push(20);
     q.pop(); // Удаляет 10
     ```

3. **`front()`**:
   - Возвращает элемент в начале очереди.
   - **Важно**: Не вызывайте `front()` на пустой очереди, это приведет к неопределенному поведению.
   - Пример:
     ```cpp
     std::queue<int> q;
     q.push(10);
     q.push(20);
     std::cout << q.front(); // 10
     ```

4. **`back()`**:
   - Возвращает элемент в конце очереди.
   - **Важно**: Не вызывайте `back()` на пустой очереди, это приведет к неопределенному поведению.
   - Пример:
     ```cpp
     std::queue<int> q;
     q.push(10);
     q.push(20);
     std::cout << q.back(); // 20
     ```

5. **`empty()`**:
   - Проверяет, пуста ли очередь.
   - Возвращает `true`, если очередь пуста, и `false` в противном случае.
   - Пример:
     ```cpp
     std::queue<int> q;
     if (q.empty()) {
         std::cout << "Queue is empty.";
     }
     ```

6. **`size()`**:
   - Возвращает количество элементов в очереди.
   - Пример:
     ```cpp
     std::queue<int> q;
     q.push(10);
     q.push(20);
     std::cout << q.size(); // 2
     ```

---

### Пример использования всех методов

```cpp
#include <iostream>
#include <queue>

int main() {
    std::queue<int> q;

    // Добавляем элементы
    q.push(10);
    q.push(20);
    q.push(30);

    // Проверяем размер очереди
    std::cout << "Size: " << q.size() << std::endl; // 3

    // Доступ к первому элементу
    std::cout << "Front element: " << q.front() << std::endl; // 10

    // Доступ к последнему элементу
    std::cout << "Back element: " << q.back() << std::endl; // 30

    // Удаляем первый элемент
    q.pop();
    std::cout << "Front element after pop: " << q.front() << std::endl; // 20

    // Проверяем, пуста ли очередь
    if (!q.empty()) {
        std::cout << "Queue is not empty." << std::endl;
    }

    // Удаляем все элементы
    while (!q.empty()) {
        std::cout << q.front() << " ";
        q.pop();
    }
    std::cout << std::endl;

    // Проверяем, пуста ли очередь
    if (q.empty()) {
        std::cout << "Queue is empty." << std::endl;
    }

    return 0;
}
```

**Вывод:**
```
Size: 3
Front element: 10
Back element: 30
Front element after pop: 20
Queue is not empty.
20 30 
Queue is empty.
```

---

### Использование другого контейнера для `std::queue`

По умолчанию `std::queue` использует `std::deque` для хранения элементов. Однако можно указать другой контейнер, например `std::list`.

#### Пример с использованием `std::list`:
```cpp
#include <iostream>
#include <queue>
#include <list>

int main() {
    std::queue<int, std::list<int>> q;

    q.push(10);
    q.push(20);
    q.push(30);

    while (!q.empty()) {
        std::cout << q.front() << " "; // 10 20 30
        q.pop();
    }

    return 0;
}
```

**Вывод:**
```
10 20 30 
```

---

### Итог

- `std::queue` предоставляет интерфейс очереди (FIFO).
- Основные методы: `push`, `pop`, `front`, `back`, `empty`, `size`.
- По умолчанию используется `std::deque`, но можно указать другой контейнер.
- Используйте `std::queue`, когда нужен простой и эффективный интерфейс очереди.