#include "binary_tree_level_order_traversal.h"

#include <iostream>
#include <format>
#include <queue>

namespace binary_tree_level_order_traversal {

struct TreeNode {
    int value;
    TreeNode* left;
    TreeNode* right;

    explicit TreeNode(const int value):
        value(value),
        left(nullptr),
        right(nullptr) {}
};

std::vector<std::vector<int>> level_order(const TreeNode* const node) {
    std::vector<std::vector<int>> result;
    if (!node) return result;

    std::queue<const TreeNode*> q;
    q.push(node);

    while (!q.empty()) {
        const int level_size{static_cast<int>(q.size())};
        std::vector<int> level;
        level.reserve(level_size);

        for (int i{}; i < level_size; ++i) {
            const auto current = q.front();
            q.pop();
            level.push_back(current->value);

            if (current->left) q.push(current->left);
            if (current->right) q.push(current->right);
        }
        result.push_back(std::move(level));
    }

    return result;
}

void dfs_helper(const TreeNode* const node, const int depth, std::vector<std::vector<int>>& result) {
    if (!node) return;

    if (depth == static_cast<int>(result.size())) {
        result.push_back({});
    }
    result[depth].push_back(node->value);

    dfs_helper(node->left, depth + 1, result);
    dfs_helper(node->right, depth + 1, result);
}

std::vector<std::vector<int>> level_order_dfs(const TreeNode* const node) {
    std::vector<std::vector<int>> result;
    dfs_helper(node, 0, result);

    return result;
}

static void delete_tree(const TreeNode* const node) {
    if (node == nullptr) return;

    delete_tree(node->left);
    delete_tree(node->right);

    delete node;
}

void demo() {
    const auto root = new TreeNode(15);
    root->left = new TreeNode(10);
    root->right = new TreeNode(20);
    root->left->left = new TreeNode(9);
    root->right->left = new TreeNode(18);
    root->right->right = new TreeNode(25);

    for (const auto result = level_order(root); auto vec : result) {
        std::cout << "[";
        std::string delimiter;
        for (const auto item: vec) {
            std::cout << std::format("{}{}", delimiter, item);
            delimiter = ", ";
        }
        std::cout << "]\n";
    }

    for (const auto result = level_order_dfs(root); auto vec : result) {
        std::cout << "[";
        std::string delimiter;
        for (const auto item: vec) {
            std::cout << std::format("{}{}", delimiter, item);
            delimiter = ", ";
        }
        std::cout << "]\n";
    }

    delete_tree(root);
}

}
