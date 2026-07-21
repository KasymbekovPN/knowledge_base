[[raw data/cpp/interview/_|<=]]

## Remove Nth Node From End of List

**Условие:** дан односвязный список и число `n`. Удалить `n`-й узел с конца списка и вернуть голову. Гарантируется, что `n` валидно (`1 <= n <= длина списка`).

### Идея

Наивно — сначала пройти список и посчитать длину `L`, затем пройти второй раз до узла `L - n` (перед удаляемым) и удалить следующий: два прохода, O(n) времени, но два раза по списку. Оптимально — **один проход двумя указателями с фиксированным зазором**: продвигаем `fast` на `n` шагов вперёд от старта, затем двигаем `slow` и `fast` одновременно, пока `fast` не дойдёт до конца. К этому моменту `slow` окажется ровно перед узлом, который нужно удалить.

Чтобы не писать отдельно случай удаления **головы** списка (когда `n` равно длине списка), используем dummy-узел перед головой.

### Решение

```cpp
#include "remove_nth_node_from_end.h"  
  
#include <iostream>  
#include <format>  
#include <ranges>  
#include <algorithm>  
#include <vector>  
  
namespace remove_nth_node_from_end {  
  
struct Node {  
    int value;  
    Node* next;  
    explicit Node(const int value):  
        value(value),  
        next(nullptr) {}  
};  
  
Node* do_remove_nth_node_from_end(Node* head, const int n) {  
    Node dummy{0};  
    dummy.next = head;  
  
    const Node* fast = &dummy;  
    Node* slow = &dummy;  
  
    // сдвигаем fast на n шагов вперёд  
    for (int i{}; i < n; ++i) {  
        fast = fast->next;  
    }  
    // двигаем оба, пока fast не дойдёт до последнего узла  
    while (fast->next != nullptr) {  
        fast = fast->next;  
        slow = slow->next;  
    }  
    // slow сейчас стоит перед узлом, который нужно удалит  
    const Node* to_delete = slow->next;  
    slow->next = to_delete->next;  
    delete to_delete;  
  
    return dummy.next;  
}  
  
static Node* create_sorted_list(std::vector<int> init_list) {  
    if (init_list.empty()) {  
        return nullptr;  
    }  
    std::ranges::sort(init_list);  
    const auto head = new Node{init_list.at(0)};  
    auto current_node = head;  
  
    for (int i{1}; i < static_cast<int>(init_list.size()); ++i) {  
        const auto node = new Node{init_list.at(i)};  
        current_node->next = node;  
        current_node = node;  
    }  
    return head;  
}  
  
static void print_list(Node* head) {  
    if (head == nullptr) {  
        std::cout << "END\n";  
        return;  
    }  
    std::cout << std::format("{} ", head->value);  
    print_list(head->next);  
}  
  
void demo() {  
    const auto l0 = create_sorted_list(std::vector<int>{1, 2, 3, 4, 5});  
    print_list(l0);  
  
    const auto result = do_remove_nth_node_from_end(l0, 3);  
    print_list(result);  
}  
  
}
```

### Разбор

- `dummy.next = head` — фиктивный узел перед головой; и `fast`, и `slow` стартуют с него, а не с самого `head`. Это позволяет единообразно обработать случай, когда удаляется первый узел списка (`n == длина списка`) — `slow` окажется на `dummy`.
- Первый цикл сдвигает `fast` на `n` шагов вперёд — теперь между `fast` и `slow` зазор ровно `n` узлов.
- Второй цикл двигает оба указателя одновременно, пока `fast` не окажется на **последнем** узле (`fast->next == nullptr`). Из-за зазора в `n`, к этому моменту `slow` стоит ровно перед узлом, отстоящим от конца на `n` позиций — то есть перед тем, который нужно удалить.
- `slow->next = toDelete->next` — стандартное удаление узла из односвязного списка: перепривязываем указатель, минуя удаляемый узел.
- `delete toDelete` — освобождаем память (если список управляется сырыми указателями, как в LeetCode-стиле).

### Пример

```
Список: 1 -> 2 -> 3 -> 4 -> 5,  n = 2

dummy -> 1 -> 2 -> 3 -> 4 -> 5
fast = dummy, slow = dummy

сдвиг fast на n=2: fast=dummy->1->2  (fast теперь на узле "2")

цикл, пока fast->next != nullptr:
  fast(2)->next=3 -> fast=3, slow=1
  fast(3)->next=4 -> fast=4, slow=2
  fast(4)->next=5 -> fast=5, slow=3
  fast(5)->next=nullptr -> стоп

slow стоит на узле "3", slow->next = узел "4" (это n=2-й с конца)
toDelete = 4
slow->next = 5 (пропускаем узел 4)

Результат: 1 -> 2 -> 3 -> 5
```

### Сложность

- Время: **O(L)** — где `L` — длина списка, один проход (плюс начальный сдвиг на `n`, что не превышает `L`).
- Память: **O(1)**.

### Частые вариации

- **Middle of the Linked List** — тот же приём slow/fast pointers, но без начального зазора — просто `fast` идёт вдвое быстрее `slow`.
- **Remove Duplicates from Sorted List** — другой паттерн: сравнение соседних узлов и удаление совпадающих, без двух указателей с зазором.
- **Swap Nodes in Pairs** — манипуляция указателями попарно, тоже часто с dummy-узлом для унификации краевых случаев.
