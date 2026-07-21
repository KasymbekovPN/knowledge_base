#pragma once

namespace lowest_common_ancestor {
    struct TreeNode;
    TreeNode* lowest_common_ancestor_bst(TreeNode* root, TreeNode* p, TreeNode* q);
    TreeNode* lowest_common_ancestor(TreeNode* root, TreeNode* p, TreeNode* q);
    void demo();
}