#pragma once

namespace reverse_linked_list {
    struct Node {
        int value;
        Node* next;
        explicit Node(const int value):
            value{value},
            next{nullptr} {}
    };
    Node* reverse_list_it(Node* head);
    Node* reverse_list_re(Node* head);
    void demo();
}
