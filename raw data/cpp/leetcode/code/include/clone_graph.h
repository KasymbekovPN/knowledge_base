#pragma once

#include <unordered_map>

namespace clone_graph {
    struct Node;
    Node* clone_graph_dfs(Node* node);
    Node* clone_graph_dfs_impl(Node* node, std::unordered_map<Node*, Node*>& visited);
    Node* clone_graph_bfs(Node* node);
    void demo();
}