#pragma once

namespace max_depth_binary_tree {
    struct TreeNode;
    int max_depth_recursive(const TreeNode*);
    int max_depth_bfs(const TreeNode*);
    int max_depth_dfs(const TreeNode*);
    void demo();
}