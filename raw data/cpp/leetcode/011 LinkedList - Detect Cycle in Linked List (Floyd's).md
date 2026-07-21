[[raw data/cpp/interview/_|<=]]

## Detect Cycle in Linked List (Floyd's algorithm)

**Условие:** дан односвязный список. Определить, есть ли в нём цикл (какой-то узел, начиная с которого, `next` в конце концов возвращается к уже посещённому узлу).

### Идея

Наивно — хранить множество посещённых узлов (адресов) и проверять на каждом шаге: O(n) по памяти. Оптимальный подход без доп. памяти — **алгоритм Флойда ("черепаха и заяц")**: два указателя, один (`slow`) двигается на 1 шаг за итерацию, второй (`fast`) — на 2 шага. Если цикла нет, `fast` дойдёт до `nullptr`. Если цикл есть, `fast` рано или поздно "догонит" `slow` внутри цикла — они встретятся в одной и той же точке.

Почему они обязательно встретятся: внутри цикла расстояние между `fast` и `slow` на каждом шаге сокращается на 1 (относительно друг друга `fast` быстрее на 1 позицию за итерацию) — значит рано или поздно расстояние станет равным 0 по модулю длины цикла.

### Решение

```cpp
#include "detect_cycle_in_ll.h"  
  
#include <iostream>  
#include <format>  
  
namespace detect_cycle_in_ll {  
  
bool has_cycle(Node* head) {  
    Node* slow{head};  
    Node* fast{head};  
  
    while (fast != nullptr && fast->next != nullptr) {  
        slow = slow->next;  
        fast = fast->next->next;  
  
        if (slow == fast) return true;  
    }  
    return false;  
}  
  
Node* detect_cycle_start(Node* head) {  
    Node* slow{head};  
    Node* fast{head};  
  
    while (fast != nullptr && fast->next != nullptr) {  
        slow = slow->next;  
        fast = fast->next->next;  
  
        if (slow == fast) {  
            Node* ptr{head};  
            while (ptr != slow) {  
                ptr = ptr->next;  
                slow = slow->next;  
            }  
            return ptr;  
        }
	}  
    return nullptr;  
}  
  
void demo() {  
    constexpr int START_VALUE{42};  
  
    Node* head = new Node{START_VALUE};  
    Node* current{head};  
    Node* catch_node{nullptr};  
    for (int i{}; i < 10; ++i) {  
        Node* n = new Node{START_VALUE + i};  
        current->next = n;  
        current = n;  
        if (constexpr int CATCH_INDEX{4}; CATCH_INDEX == i) {  
            catch_node = current;  
        }
	}
	current->next = catch_node;  
  
    std::cout << std::format("Has cycle: {}\n", (has_cycle(head)) ? "true" : "false");  
    if (const Node* cycle_start = detect_cycle_start(head); cycle_start) {  
        std::cout << std::format("Cycle start: {}", cycle_start->value);  
    } else {  
        std::cout << "Cycle start does not detected.\n";  
    }
}  
  
}
```

### Разбор

- Условие `fast != nullptr && fast->next != nullptr` защищает от разыменования `nullptr`: `fast` двигается на 2 шага, поэтому нужно проверить оба — сам узел и следующий за ним.
- `slow` двигается на 1 шаг, `fast` — на 2. Если цикла нет, `fast` первым дойдёт до конца списка и цикл завершится по условию.
- Как только `slow == fast` (совпали указатели, не значения!) — значит оба находятся внутри цикла и встретились.

### Пример

```
Список с циклом: 1 -> 2 -> 3 -> 4 -> 5 -> (снова на 3)

slow=1, fast=1
шаг1: slow=2, fast=3
шаг2: slow=3, fast=5
шаг3: slow=4, fast=4 (fast: 5->3->4)  -> slow==fast -> true
```

### Сложность

- Время: **O(n)** — в худшем случае указатели проходят список плюс цикл линейное число раз.
- Память: **O(1)** — только два указателя, в отличие от подхода с hash set (O(n)).

### Частый доп. вопрос: найти узел, с которого начинается цикл

Классическое продолжение задачи (LeetCode 142). После обнаружения встречи ставим один указатель обратно в `head`, второй оставляем в точке встречи, и двигаем оба **на 1 шаг** одновременно — они встретятся ровно в начале цикла.


**Почему это работает (доказательство математикой):** пусть расстояние от `head` до начала цикла — `a`, от начала цикла до точки встречи — `b`, оставшаяся часть цикла до начала — `c` (длина цикла = `b + c`). К моменту встречи `slow` прошёл `a + b`, `fast` прошёл `a + b + k(b+c)` для некоторого целого `k`, и поскольку `fast` прошёл вдвое больше шагов: `2(a+b) = a + b + k(b+c)` → `a + b = k(b+c)` → `a = k(b+c) - b = (k-1)(b+c) + c`. Значит расстояние `a` от головы до старта цикла равно (с точностью до полных обходов цикла) расстоянию `c` от точки встречи до старта цикла — поэтому одновременное движение с шагом 1 из `head` и из точки встречи сходится ровно в начале цикла.

### Частые вариации

- **Linked List Cycle II** — разобрано выше (поиск точки начала цикла).
- **Happy Number** — тот же алгоритм Флойда применяется не к списку, а к последовательности чисел (сумма квадратов цифр), где "next" — функция преобразования числа.
- **Find the Duplicate Number** — задача сводится к поиску цикла в "функциональном графе", построенном по массиву как отображению индексов.
