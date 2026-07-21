[[raw data/cpp/interview/_|<=]]

## Reverse Linked List

**Условие:** дан односвязный список (head). Развернуть список и вернуть новую голову.

### Идея

Нужно перенаправить у каждого узла указатель `next`, чтобы он указывал на предыдущий узел вместо следующего. Идём по списку, на каждом шаге запоминаем следующий узел (иначе потеряем к нему доступ после разворота указателя), разворачиваем текущий `next`, сдвигаем указатели вперёд.

### Решение

```cpp
#include "reverse_linked_list.h"  
  
#include <iostream>  
#include <format>  
  
namespace reverse_linked_list {  
  
Node* reverse_list_it(Node* head) {  
    Node* prev{nullptr};  
    Node* current{head};  
  
    while (current != nullptr) {  
        Node* next{current->next};  
        current->next = prev;  
        prev = current;  
        current = next;  
    }  
    return prev;  
}  
  
Node* reverse_list_re(Node* head) {  
    if (head == nullptr || head->next == nullptr) {  
        return head;  
    }  
    Node* new_head{reverse_list_re(head->next)};  
    head->next->next = head;  
    head->next = nullptr;  
  
    return new_head;  
}  
  
static void print_list(const Node* head) {  
    if (head == nullptr) {  
        std::cout << "NULL\n";  
        return;  
    }  
    std::cout << std::format("value: {} -> ", head->value);  
    print_list(head->next);  
}  
  
static Node* create(const int base, const int size) {  
    Node* head = new Node{base};  
    Node* current{head};  
    for (int i{}; i < size; ++i) {  
        Node* n = new Node{base + i};  
        current->next = n;  
        current = n;  
    }  
    return head;  
}  
  
void demo() {  
    constexpr int START_VALUE{42};  
  
    const auto n0 = create(START_VALUE, 5);  
    print_list(n0);  
    const auto r0 = reverse_list_it(n0);  
    print_list(r0);  
  
    const auto n1 = create(START_VALUE, 5);  
    print_list(n1);  
    const auto r1 = reverse_list_re(n1);  
    print_list(r1);  
}  
  
}
```

### Разбор

- `prev` изначально `nullptr` — это будущий `next` для исходного первого узла (он станет последним и должен указывать на `nullptr`).
- `next = cur->next` — обязательно сохраняем **до** изменения `cur->next`, иначе потеряем связь с остальной частью списка.
- `cur->next = prev` — собственно разворот связи текущего узла.
- Цикл завершается, когда `cur == nullptr` — на этот момент `prev` указывает на последний обработанный узел, то есть на новую голову.

### Пример

```
Исходный список: 1 -> 2 -> 3 -> nullptr

prev=nullptr, cur=1
  next=2; 1->next=nullptr; prev=1; cur=2
  list: nullptr <- 1    2 -> 3 -> nullptr

prev=1, cur=2
  next=3; 2->next=1; prev=2; cur=3
  list: nullptr <- 1 <- 2    3 -> nullptr

prev=2, cur=3
  next=nullptr; 3->next=2; prev=3; cur=nullptr
  list: nullptr <- 1 <- 2 <- 3

cur == nullptr -> стоп, return prev (узел 3)

Результат: 3 -> 2 -> 1 -> nullptr
```

### Сложность

- Время: **O(n)** — один проход.
- Память: **O(1)** — три указателя, без рекурсии и доп. структур.

Разбор: рекурсия спускается до последнего узла (он становится `newHead` и больше не меняется на обратном пути). На каждом уровне возврата разворачиваем связь между `head` и `head->next`: `head->next->next = head` делает следующий узел указывающим обратно, `head->next = nullptr` разрывает старую связь вперёд (важно на последнем уровне рекурсии, иначе получится цикл).

- Время: **O(n)**.
- Память: **O(n)** — глубина стека рекурсии.

### Частые вариации

- **Reverse Linked List II** — развернуть только подотрезок `[left, right]`, остальное оставить как есть.
- **Reverse Nodes in k-Group** — развернуть список группами по `k` узлов.
- **Palindrome Linked List** — найти середину (slow/fast pointers), развернуть вторую половину, сравнить с первой.
