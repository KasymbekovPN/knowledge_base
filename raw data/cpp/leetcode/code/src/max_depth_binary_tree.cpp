#include "max_depth_binary_tree.h"

#include <iostream>
#include <format>
#include <algorithm>
#include <queue>
#include <stack>
#include <utility>

namespace max_depth_binary_tree {

struct TreeNode {
    int value;
    TreeNode* left;
    TreeNode* right;
    explicit TreeNode(const int value):
        value(value),
        left(nullptr),
        right(nullptr) {}
};

int max_depth_recursive(const TreeNode* node) {
    if (node == nullptr) return 0;

    return 1 + std::max(
        max_depth_recursive(node->left),
        max_depth_recursive(node->right));
}

int max_depth_bfs(const TreeNode* node) {
    if (node == nullptr) return 0;

    std::queue<const TreeNode*> q;
    q.push(node);
    int depth{};

    while (!q.empty()) {
        const int level_size{static_cast<int>(q.size())};
        for (int i{}; i < level_size; ++i) {
            const TreeNode* current{q.front()};
            q.pop();

            if (current->left) q.push(current->left);
            if (current->right) q.push(current->right);
        }
        ++depth;
    }

    return depth;
}

int max_depth_dfs(const TreeNode* node) {
    if (node == nullptr) return 0;

    std::stack<std::pair<const TreeNode*, int>> stk;
    stk.emplace(node, 1);
    int best{};

    while (!stk.empty()) {
        auto [current, depth] = stk.top();
        stk.pop();

        best = std::max(best, depth);

        if (current->left) stk.emplace(current->left, depth + 1);
        if (current->right) stk.emplace(current->right, depth + 1);
    }

    return best;
}

static void delete_tree(const TreeNode* node) {
    if (node == nullptr) return;

    delete_tree(node->left);
    delete_tree(node->right);

    delete node;
}

void demo() {
    const auto root = new TreeNode(3);
    root->left = new TreeNode(9);
    root->right = new TreeNode(20);
    root->right->left = new TreeNode(15);
    root->right->right = new TreeNode(7);

    std::cout << std::format("REC {}\n", max_depth_recursive(root));
    std::cout << std::format("BFS {}\n", max_depth_bfs(root));
    std::cout << std::format("DFS {}\n", max_depth_dfs(root));

    delete_tree(root);
}

}