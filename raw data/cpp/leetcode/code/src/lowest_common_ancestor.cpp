#include "lowest_common_ancestor.h"

#include <iostream>
#include <format>

namespace lowest_common_ancestor {

struct TreeNode {
    int value;
    TreeNode* left;
    TreeNode* right;
    explicit TreeNode(const int value):
        value(value),
        left(nullptr),
        right(nullptr) {}
};

TreeNode* lowest_common_ancestor_bst(TreeNode* root, TreeNode* p, TreeNode* q) {
    TreeNode* node{root};

    while (node) {
        if (p->value < node->value && q->value < node->value) {
            node = node->left;
        } else if (p->value > node->value && q->value > node->value) {
            node = node->right;
        } else {
            return node;
        }
    }

    return nullptr;
}

TreeNode* lowest_common_ancestor(TreeNode* root, TreeNode* p, TreeNode* q) {
    if (root == nullptr || root == p || root == q) return root;

    TreeNode* left = lowest_common_ancestor(root->left, p, q);
    TreeNode* right = lowest_common_ancestor(root->right, p, q);
    if (left && right) {
        return root;
    }

    return left ? left : right;
}

static void print_node(const TreeNode* const node) {
    if (node) {
        std::cout << std::format("[value: {}]\n", node->value);
    } else {
        std::cout << "NULL\n";
    }
}

static void delete_tree(const TreeNode* const node) {
    if (node == nullptr) return;

    delete_tree(node->left);
    delete_tree(node->right);

    delete node;
}

void demo() {
    //     6
    //    / \
    //   2   8
    //  / \ / \
    // 0  4 7  9
    //   / \
    //  3   5
    const auto root = new TreeNode(6);
    root->left = new TreeNode(2);
    root->right = new TreeNode(8);
    const auto node0 = new TreeNode(0);
    root->left->left = node0;
    root->left->right = new TreeNode(4);
    root->right->left = new TreeNode(7);
    root->right->right = new TreeNode(9);
    root->left->right->left = new TreeNode(3);
    const auto node5 = new TreeNode(5);
    root->left->right->right = node5;

    print_node(lowest_common_ancestor_bst(root, node0, node5));
    print_node(lowest_common_ancestor(root, node0, node5));

    delete_tree(root);
}

}