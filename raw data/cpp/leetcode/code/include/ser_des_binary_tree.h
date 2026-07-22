#pragma once

#include <string>
#include <sstream>

namespace ser_des_binary_tree {
    struct TreeNode;

    class Codec {
    public:
        std::string serialize(TreeNode* root);
        TreeNode* deserialize(const std::string& data);

        std::string serialize_bfs(TreeNode* root);
        TreeNode* deserialize_bfs(const std::string& data);
    private:
        void serialize_helper(TreeNode* root, std::ostringstream& out);
        TreeNode* deserialize_helper(std::istringstream& in);
    };

    void demo();

}
