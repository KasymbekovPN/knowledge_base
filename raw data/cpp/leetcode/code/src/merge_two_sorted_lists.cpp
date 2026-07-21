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
