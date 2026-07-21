#pragma once

#include <vector>

namespace merge_two_sorted_lists {
    struct Node;
    Node* create_sorted_list(std::vector<int> init_list);
    Node* do_merge_two_sorted_lists_it(Node*, Node*);
    Node* do_merge_two_sorted_lists_re(Node*, Node*);
    void demo();
}
