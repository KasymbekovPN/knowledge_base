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
    }
};

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
