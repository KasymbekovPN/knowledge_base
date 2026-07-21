[[raw data/cpp/interview/_|<=]]

## Merge Two Sorted Lists

**Условие:** даны два отсортированных односвязных списка `list1` и `list2`. Слить их в один отсортированный список и вернуть его голову. Разрешается переиспользовать узлы исходных списков (новые узлы создавать не нужно).

### Идея

Классическое слияние, как в merge sort. Идём по обоим спискам одновременно, на каждом шаге сравниваем текущие головы `list1` и `list2`, меньший узел отцепляем и присоединяем к результату, продвигая соответствующий указатель. Чтобы не писать отдельную обработку "первого" узла результата, используем **dummy-узел** (фиктивную голову) — это убирает специальные случаи при старте.

### Решение

```cpp
#include "merge_two_sorted_lists.h"  
  
#include <algorithm>  
#include <iostream>  
#include <format>  
#include <ranges>  
  
namespace merge_two_sorted_lists {  
  
struct Node {  
    int value;  
    Node* next;  
    explicit Node(const int value):  
        value(value),  
        next(nullptr) {}  
};  
  
Node* create_sorted_list(std::vector<int> init_list) {  
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
  
Node* do_merge_two_sorted_lists_it(Node* l0, Node* l1) {  
    Node dummy{0};  
    Node* tail = &dummy;  
  
    while (l0 != nullptr && l1 != nullptr) {  
        if (l0->value <= l1->value) {  
            tail->next = l0;  
            l0 = l0->next;  
        } else {  
            tail->next = l1;  
            l1 = l1->next;  
        }
		tail = tail->next;  
    }  
    // один из списков закончился раньше — присоединяем остаток другого целиком  
    tail->next = (l0 != nullptr) ? l0 : l1;  
  
    return dummy.next;  
}  
  
Node* do_merge_two_sorted_lists_re(Node* l0, Node* l1) {  
    if (l0 == nullptr) return l1;  
    if (l1 == nullptr) return l0;  
  
    if (l0->value <= l1->value) {  
        l0->next = do_merge_two_sorted_lists_re(l0->next, l1);  
        return l0;  
    }  
    l1->next = do_merge_two_sorted_lists_re(l0, l1->next);  
    return l1;  
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
    auto l0 = create_sorted_list(std::vector<int>{1, 2, 3, 4, 5});  
    auto l1 = create_sorted_list(std::vector<int>{10, 8, 6, 4, 2, 0});  
    const auto result_it = do_merge_two_sorted_lists_it(l0, l1);  
    print_list(result_it);  
  
    auto l2 = create_sorted_list(std::vector<int>{1, 2, 3, 4, 5});  
    auto l3 = create_sorted_list(std::vector<int>{10, 8, 6, 4, 2, 0});  
    const auto result_re = do_merge_two_sorted_lists_re(l2, l3);  
    print_list(result_re);  
}  
  
}
```

### Разбор

- `dummy` — фиктивный узел на стеке, его `next` в итоге будет указывать на настоящую голову результата. Это избавляет от отдельной логики "куда присвоить первый узел".
- `tail` — указатель на последний узел уже построенного результата; на каждом шаге присоединяем к нему меньший из текущих узлов и сдвигаем `tail` вперёд.
- `list1->val <= list2->val` — при равенстве берём из `list1`, это делает сортировку стабильной (не влияет на корректность, но иногда важно по условию задачи).
- Когда один из списков полностью пройден, второй список уже отсортирован — его можно целиком присоединить одним присваиванием `tail->next = ...`, а не идти по нему поэлементно.
- Узлы не создаются заново — переиспользуются существующие, поэтому доп. память не тратится на сами данные списка.

### Пример

```
list1: 1 -> 2 -> 4
list2: 1 -> 3 -> 4

dummy -> ...
1<=1 -> взять list1(1); list1=2
dummy->1
1<=3? list2(1)<=list1(2) -> взять list2(1); list2=3
dummy->1->1
2<=3 -> взять list1(2); list1=4
dummy->1->1->2
4<=3? нет -> взять list2(3); list2=4
dummy->1->1->2->3
4<=4 -> взять list1(4); list1=nullptr
dummy->1->1->2->3->4
list1 == nullptr -> цикл завершён
tail->next = list2 (оставшийся 4)

Результат: 1 -> 1 -> 2 -> 3 -> 4 -> 4
```

### Сложность

- Время: **O(n + m)** — где `n`, `m` — длины списков, каждый узел обрабатывается ровно один раз.
- Память: **O(1)** доп. памяти (не считая самого результата, который переиспользует существующие узлы) — итеративная версия.

### Рекурсивное решение (альтернатива)

Компактнее, но требует O(n+m) памяти под стек рекурсии — частый доп. вопрос: "а можно без рекурсии?".

### Частые вариации

- **Merge k Sorted Lists** — обобщение на `k` списков: приоритетная очередь (min-heap) по значениям голов списков, O(N log k), либо попарное слияние по схеме divide & conquer.
- **Sort List** — отсортировать один несортированный список; классическое решение — merge sort на связном списке (найти середину через slow/fast pointers + это же слияние).
- **Add Two Numbers** — похожая механика слияния, но списки представляют числа по цифрам, и нужно учитывать перенос разряда.
