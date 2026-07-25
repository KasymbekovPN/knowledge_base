[[raw data/cpp/interview/_|<=]]

## Merge K Sorted Lists

**Условие:** дан массив из `k` отсортированных односвязных списков. Слить их все в один отсортированный список и вернуть его голову.

### Идея

Обобщение задачи Merge Two Sorted Lists (уже разобрана) на `k` списков. Два основных подхода:

1. **Min-heap (приоритетная очередь)** — на каждом шаге нужно выбрать минимальный элемент среди текущих голов всех `k` списков. Куча даёт это за O(log k) вместо линейного перебора всех `k` голов (O(k)) на каждом шаге.
2. **Divide & Conquer (попарное слияние)** — сливаем списки попарно, как в merge sort: `k` списков → `k/2` списков → `k/4` → ... → 1 список. Каждое слияние использует обычный `mergeTwoLists`.

```cpp
#include "merge_k_sorted_lists.h"  
  
#include <vector>  
#include <queue>  
#include <iostream>  
#include <format>  
  
namespace merge_k_sorted_lists {  
  
struct ListNode {  
    inline static int counter{0};  
  
    int id;  
    int value;  
    ListNode* next;  
    explicit ListNode(const int value):  
        id{counter++},  
        value{value},  
        next{nullptr} {}  
};  
  
struct Compare {  
    bool operator()(const ListNode* lhs, const ListNode *rhs) const {  
        return lhs->value > rhs->value;  
    }};  
  
static ListNode* create_list(const int base, const int size) {  
    ListNode dummy{0};  
    dummy.next = new ListNode(base);  
    auto current = dummy.next;  
    for (int i{1}; i < size; ++i) {  
        const auto next = new ListNode(base + i);  
        current->next = next;  
        current = current->next;  
    }  
    return dummy.next;  
}  
  
static void delete_list(const ListNode* const node) {  
    if (!node) return;  
  
    delete_list(node->next);  
    delete node;  
}  
  
static void print_list(const ListNode* const node, const std::string& label) {  
    if (!node) {  
        std::cout << "END\n";  
        return;  
    }  
    std::cout << (label == "" ? label : "[" + label + "] ") << node->value << "(" << node->id << ") ";  
    print_list(node->next, "");  
}  
  
// min-heap solution  
ListNode* merge_lists(const std::vector<ListNode*>& lists) {  
    std::priority_queue<ListNode*, std::vector<ListNode*>, Compare> min_heap;  
  
    for (auto* node : lists) {  
        if (node) {  
            min_heap.push(node);  
        }    
    }  
    ListNode dummy{0};  
    ListNode* tail = &dummy;  
  
    while (!min_heap.empty()) {  
        ListNode* smallest = min_heap.top();  
        min_heap.pop();  
  
        tail->next = smallest;  
        tail = tail->next;  
  
        if (smallest->next) {  
            min_heap.push(smallest->next);  
        }    
    }  
    return dummy.next;  
}  
  
// divide & conquer solution  
ListNode* merge_two_lists(ListNode* lhs, ListNode* rhs) {  
    ListNode dummy{0};  
    ListNode* tail = &dummy;  
  
    while (lhs && rhs) {  
        if (lhs->value <= rhs->value) {  
            tail->next = lhs;  
            lhs = lhs->next;  
        } else {  
            tail->next = rhs;  
            rhs = rhs->next;  
        }        
        tail = tail->next;  
    }    
    tail->next = lhs ? lhs : rhs;  
  
    return dummy.next;  
}  
  
ListNode* merge_lists_dc(std::vector<ListNode*>& lists) {  
    if (lists.empty()) return nullptr;  
  
    const int N{static_cast<int>(lists.size())};  
    int interval{1};  
  
    while (interval < N) {  
        for (int i{}; i + interval < N; i += interval*2) {  
            lists[i] = merge_two_lists(lists[i], lists[i + interval]);  
        }        
        interval *= 2;  
    }  
    return lists[0];  
}  
  
void demo() {  
    const auto lists = std::vector<ListNode*>{  
        create_list(5, 5), create_list(7, 5), create_list(10, 5)};  
    for (const auto* node : lists) {  
        print_list(node, "list");  
    }  
    const auto result = merge_lists(lists);  
    print_list(result, "result");  
    delete_list(result);  
  
    auto lists_dc = std::vector<ListNode*>{  
        create_list(5, 5), create_list(7, 5), create_list(10, 5)};  
  
    const auto result_dc = merge_lists_dc(lists_dc);  
    print_list(result_dc, "result_dc");  
    delete_list(result_dc);  
}  
}
```

### Решение 1: Min-heap

### Разбор

- `Compare` — кастомный компаратор для `priority_queue`: по умолчанию `priority_queue` — max-heap, поэтому нужно инвертировать сравнение (`a->val > b->val` означает "a менее приоритетен", что при инверсии даёт min-heap по значению).
- Изначально в кучу кладём **головы** всех непустых списков — по одному узлу от каждого.
- На каждой итерации извлекаем минимальный узел из кучи (это гарантированно минимум среди всех текущих "кандидатов"), присоединяем его к результату (dummy + tail — та же техника, что и в Merge Two Sorted Lists), и если у извлечённого узла есть `next` — добавляем этот `next` в кучу как нового кандидата от этого же списка.
- Куча в любой момент содержит не более `k` элементов (по одному от каждого ещё не исчерпанного списка), поэтому каждая операция push/pop — O(log k).

### Пример

```
lists = [1->4->5, 1->3->4, 2->6]

heap изначально: {1(из списка1), 1(из списка2), 2}

extract min=1(список1): tail->1; push(4 из списка1)
  heap: {1(список2), 2, 4}
extract min=1(список2): tail->1->1; push(3 из списка2)
  heap: {2, 3, 4}
extract min=2: tail->...->2; push(6 из списка3)
  heap: {3, 4, 6}
extract min=3: tail->...->3; push(4 из списка2)
  heap: {4, 4, 6}
extract min=4: tail->...->4; push(5 из списка1)
  heap: {4, 5, 6}
extract min=4: tail->...->4; следующего нет
  heap: {5, 6}
extract min=5: tail->...->5
  heap: {6}
extract min=6: tail->...->6
  heap: {}

Результат: 1->1->2->3->4->4->5->6
```

### Сложность

- Время: **O(N log k)**, где `N` — суммарное количество узлов во всех списках, `k` — количество списков. Каждый из `N` узлов проходит через кучу ровно один раз, каждая операция — O(log k) (размер кучи не превышает `k`).
- Память: **O(k)** — под кучу (не считая памяти под сам результат, переиспользующий существующие узлы).

### Решение 2: Divide & Conquer (попарное слияние)

### Разбор

- Каждый раунд сливает списки попарно: список `i` со списком `i + interval`. После первого раунда (`interval=1`) все пары соседних списков слиты, `interval` удваивается до 2 — теперь сливаются результаты этих пар, и так далее.
- Это в точности схема **merge sort** снизу вверх, применённая не к элементам массива, а к целым спискам.
- `lists[i] = mergeTwoLists(...)` — результат слияния сохраняется на месте `i`, освобождая позицию `i + interval` от повторного использования.
- Цикл продолжается, пока `interval < n` — на последней итерации все списки слиты в один, лежащий в `lists[0]`.

### Пример

```
lists = [L0, L1, L2, L3, L4]  (n=5)

interval=1:
  i=0: lists[0]=merge(L0,L1)
  i=2: lists[2]=merge(L2,L3)
  i=4: i+1=5 >= n -> цикл не выполняется для i=4
  lists = [merge(L0,L1), L1, merge(L2,L3), L3, L4]
interval=2:
  i=0: lists[0]=merge(lists[0], lists[2]) = merge(merge(L0,L1), merge(L2,L3))
  i=4: i+2=6>=n -> не выполняется
  lists = [merged(0,1,2,3), ..., ..., ..., L4]
interval=4:
  i=0: lists[0]=merge(lists[0], lists[4]) = все 5 списков слиты
interval=8 >= n -> стоп

Результат: lists[0]
```

### Сложность

- Время: **O(N log k)** — `log k` раундов слияния, в каждом раунде суммарно обрабатывается O(N) узлов (все списки сливаются попарно ровно один раз за раунд).
- Память: **O(1)** доп. памяти (не считая O(log k) на стек, если реализовать рекурсивно) — лучше, чем куча, так как не нужна отдельная структура данных.

### Сравнение подходов

- **Min-heap** — интуитивно понятнее, естественное обобщение "жадного выбора минимума", легко реализовать корректно с первого раза.
- **Divide & Conquer** — та же асимптотика по времени, но лучше по дополнительной памяти (без кучи), и часто быстрее на практике из-за лучшей локальности данных и меньшего оверхеда на операции с кучей.

### Частые вариации

- **Merge Two Sorted Lists** (уже разобрана) — базовый строительный блок для обоих решений.
- **Kth Smallest Element in a Sorted Matrix** — похожая идея min-heap с "продвижением" следующего кандидата из той же строки/списка.
- **Smallest Range Covering Elements from K Lists** — тоже использует min-heap по k спискам, но с доп. отслеживанием текущего максимума среди вершин кучи.
- **External Sort (сортировка данных, не помещающихся в память)** — практическое применение той же идеи min-heap слияния k отсортированных чанков с диска.
