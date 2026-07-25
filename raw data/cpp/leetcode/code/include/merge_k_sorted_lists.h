#pragma once

#include <vector>

namespace merge_k_sorted_lists {
    struct ListNode;

    // min-heap solution
    ListNode* merge_lists(const std::vector<ListNode*>&);

    // divide & conquer solution
    ListNode* merge_two_lists(ListNode*, ListNode*);
    ListNode* merge_lists_dc(std::vector<ListNode*>&);

    void demo();
}
