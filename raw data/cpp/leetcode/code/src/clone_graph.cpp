#include "clone_graph.h"

#include <unordered_map>
#include <queue>
#include <vector>
#include <iostream>
#include <format>
#include <unordered_set>

namespace clone_graph {

struct Node {
    int value;
    std::vector<Node*> neighbors;
    explicit Node(const int value):
        value(value) {}
};

Node* clone_graph_dfs(Node* node) {
    std::unordered_map<Node*, Node*> visited;
    return clone_graph_dfs_impl(node, visited);
}

Node* clone_graph_dfs_impl(Node* node, std::unordered_map<Node*, Node*>& visited) {
    if (!node) return nullptr;

    if (const auto it = visited.find(node); it != visited.end()) {
        // уже клонирован ранее -> возвращаем существующий клон
        return it->second;
    }

    const auto clone = new Node{node->value};
    visited[node] = clone;

    for (Node* neighbor : node->neighbors) {
        clone->neighbors.push_back(clone_graph_dfs_impl(neighbor, visited));
    }

    return clone;
}

Node* clone_graph_bfs(Node* node) {
    if (!node) return nullptr;

    std::unordered_map<Node*, Node*> visited;
    visited[node] = new Node{node->value};

    std::queue<Node*> q;
    q.push(node);

    while (!q.empty()) {
        Node* current{q.front()};
        q.pop();

        for (Node* neighbor : current->neighbors) {
            if (!visited.contains(neighbor)) {
                visited[neighbor] = new Node{neighbor->value};
                q.push(neighbor);
            }
            visited[current]->neighbors.push_back(visited[neighbor]);
        }
    }

    return visited[node];
}

static void print_node(const Node* const node) {
    std::cout << node << std::format(" [value: {}] (", node->value);
    for (const auto& neighbor : node->neighbors) {
        std::cout << std::format("[value: {}]", neighbor->value);
    }
    std::cout << ")\n";
}

static void collector_nodes(Node* node, std::unordered_set<Node*>& collection) {
    if (!node) return;

    if (!collection.contains(node)) {
        collection.insert(node);
        for (const auto neighbor : node->neighbors) {
            collector_nodes(neighbor, collection);
        }
        node->neighbors.clear();
    }
}

static void delete_graphs(Node* node) {
    std::unordered_set<Node*> collection;
    collector_nodes(node, collection);

    for (const auto item : collection) {
        delete item;
    }
}

void demo() {
    // Граф: 1 -- 2
    //       |    |
    //       4 -- 3
    const auto n1 = new Node{1};
    const auto n2 = new Node{2};
    const auto n3 = new Node{3};
    const auto n4 = new Node{4};

    n1->neighbors.push_back(n2);
    n1->neighbors.push_back(n4);

    n2->neighbors.push_back(n1);
    n2->neighbors.push_back(n3);

    n3->neighbors.push_back(n2);
    n3->neighbors.push_back(n4);

    n4->neighbors.push_back(n1);
    n4->neighbors.push_back(n3);

    print_node(n1);
    print_node(n2);
    print_node(n3);
    print_node(n4);

    const auto cloned_bfs = clone_graph_bfs(n1);
    print_node(cloned_bfs);

    const auto cloned_dfs = clone_graph_dfs(n1);
    print_node(cloned_dfs);

    delete_graphs(n1);
    delete_graphs(cloned_bfs);
    delete_graphs(cloned_dfs);
}

}
