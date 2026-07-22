#include "ser_des_binary_tree.h"

#include <queue>
#include <sstream>
#include <iostream>
#include <format>

namespace ser_des_binary_tree {

struct TreeNode {
    int value;
    TreeNode* left;
    TreeNode* right;
    explicit TreeNode(const int value):
        value(value),
        left(nullptr),
        right(nullptr) {}
};

std::string Codec::serialize(TreeNode* root) {
    std::ostringstream out;
    serialize_helper(root, out);

    return out.str();
}

TreeNode* Codec::deserialize(const std::string& data) {
    std::istringstream in{data};
    return deserialize_helper(in);
}

std::string Codec::serialize_bfs(TreeNode* root) {
    std::ostringstream out;
    std::queue<const TreeNode*> q;
    q.push(root);

    while (!q.empty()) {
        const auto node = q.front();
        q.pop();

        if (node) {
            out << node->value << " ";
            q.push(node->left);
            q.push(node->right);
        } else {
            out << "# ";
        }
    }

    return out.str();
}

TreeNode* Codec::deserialize_bfs(const std::string& data) {
    std::istringstream in{data};
    std::string token;
    in >> token;
    if (token == "#") return nullptr;

    const auto root = new TreeNode{std::stoi(token)};
    std::queue<TreeNode*> q;
    q.push(root);

    while (!q.empty()) {
        TreeNode* node = q.front();
        q.pop();

        if (in >> token && token != "#") {
            node->left = new TreeNode{std::stoi(token)};
            q.push(node->left);
        }
        if (in >> token && token != "#") {
            node->right = new TreeNode{std::stoi(token)};
            q.push(node->right);
        }
    }

    return root;
}

void Codec::serialize_helper(TreeNode* root, std::ostringstream& out) {
    if (!root) {
        out << "# ";
        return;
    }

    out << root->value << " ";
    serialize_helper(root->left, out);
    serialize_helper(root->right, out);
}

TreeNode* Codec::deserialize_helper(std::istringstream& in) {
    std::string token;
    in >> token;

    if (token == "#") return nullptr;

    TreeNode* node = new TreeNode{std::stoi(token)};
    node->left = deserialize_helper(in);
    node->right = deserialize_helper(in);

    return node;
}

static void delete_tree(const TreeNode* const node) {
    if (node == nullptr) return;

    delete_tree(node->left);
    delete_tree(node->right);

    delete node;
}

void demo() {
    //   1
    //  / \
    // 2   3
    //    / \
    //   4   5
    const auto root = new TreeNode{1};
    root->left = new TreeNode{2};
    root->right = new TreeNode{3};
    root->right->left = new TreeNode{4};
    root->right->right = new TreeNode{5};

    auto codec = Codec();
    std::cout << std::format("serialize: {}\n", codec.serialize(root));
    std::cout << std::format("serialize bfs: {}\n", codec.serialize_bfs(root));

    delete_tree(root);
}

}
