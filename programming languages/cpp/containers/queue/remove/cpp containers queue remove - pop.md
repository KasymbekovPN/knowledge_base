---
tags:
  - programming-language
  - cpp
  - container
  - queue
---
[[_cpp containers queue - remove|<=]]

Метод _pop()_ удаляет элемент из начала очереди.
**Важно**: `pop()` на пустой очереди приводит к неопределенному поведению.

```cpp
#include <iostream>
#include <queue>

void _test_pop(std::queue<int>&);

int main(int argc, char const *argv[]) {
    std::queue<int> q {std::deque<int>{1, 2, 3}};
    for (size_t i{}; i < 4; i++) {
        _test_pop(q);
    }

    return 0;
}

void _test_pop(std::queue<int>& q) {
    if (!q.empty()) {
        q.pop();
        std::cout << "Size: " << q.size() << std::endl;
    } else {
        std::cout << "Empty!" << std::endl;
    }
}
```

```
Size: 2
Size: 1
Size: 0
Empty!
```

---

1. **`front()`**:
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