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
