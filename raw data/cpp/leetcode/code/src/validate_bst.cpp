#include "validate_bst.h"

#include <limits>
#include <iostream>
#include <format>
#include <optional>

namespace validate_bst {

struct TreeNode {
    int value;
    TreeNode* left;
    TreeNode* right;
    explicit TreeNode(const int value):
        value(value),
        left(nullptr),
        right(nullptr) {}
};

bool is_valid(const TreeNode* const node, const long long lower, const long long upper) {
    if (!node) return true;

    if (node->value <= lower || node->value >= upper) return false;

    return is_valid(node->left, lower, node->value) &&
        is_valid(node->right, node->value, upper);
}

bool is_valid_in_order(const TreeNode* const node, std::optional<long long>& prev) {
    if (!node) return true;

    if (!is_valid_in_order(node->left, prev)) return false;

    if (prev.has_value() && node->value <= prev.value()) {
        return false;
    }
    prev = node->value;;

    return is_valid_in_order(node->right, prev);
}

static void delete_tree(const TreeNode* const node) {
    if (node == nullptr) return;

    delete_tree(node->left);
    delete_tree(node->right);

    delete node;
}

void demo() {
    const auto good_root = new TreeNode(15);
    good_root->left = new TreeNode(10);
    good_root->right = new TreeNode(20);
    good_root->left->left = new TreeNode(9);
    good_root->right->left = new TreeNode(18);
    good_root->right->right = new TreeNode(25);

    const auto bad_root = new TreeNode(10);
    bad_root->left = new TreeNode(5);
    bad_root->right = new TreeNode(15);
    bad_root->right->left = new TreeNode(6);
    bad_root->right->right = new TreeNode(20);

    std::cout << std::format("good is_valid {}\n",is_valid(
        good_root,
        std::numeric_limits<long long>::min(),
        std::numeric_limits<long long>::max()));

    std::cout << std::format("bad is_valid {}\n",is_valid(
        bad_root,
        std::numeric_limits<long long>::min(),
        std::numeric_limits<long long>::max()));

    std::optional<long long> good_prev;
    std::cout << std::format("good is_valid_in_order {}\n", is_valid_in_order(
        good_root,
        good_prev));

    std::optional<long long> bad_prev;
    std::cout << std::format("bad is_valid_in_order {}\n", is_valid_in_order(
        bad_root,
        bad_prev));

    delete_tree(good_root);
    delete_tree(bad_root);
}


}
