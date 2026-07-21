#pragma once

namespace detect_cycle_in_ll {
    struct Node {
        int value;
        Node* next;
        explicit Node(const int value):
            value(value),
            next(nullptr) {}
    };
    bool has_cycle(Node* head);
    Node* detect_cycle_start(Node* head);
    void demo();
}
