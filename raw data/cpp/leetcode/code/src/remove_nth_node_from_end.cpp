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