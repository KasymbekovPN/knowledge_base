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
