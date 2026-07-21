#pragma once

#include <optional>

namespace validate_bst {
    struct TreeNode;
    bool is_valid(const TreeNode* const node, const long long lower, const long long upper);
    bool is_valid_in_order(const TreeNode* const node, std::optional<long long>& prev);
    void demo();
}
