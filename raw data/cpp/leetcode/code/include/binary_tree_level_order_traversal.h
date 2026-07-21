#pragma once

#include <vector>

namespace binary_tree_level_order_traversal {
    struct TreeNode;
    std::vector<std::vector<int>> level_order(const TreeNode* const node);
    void dfs_helper(const TreeNode* const node, const int depth, std::vector<std::vector<int>>& result);
    std::vector<std::vector<int>> level_order_dfs(const TreeNode* const node);
    void demo();
}
