#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <algorithm>

namespace {

struct Node {
    int val;
    std::vector<Node*> neighbors;
};

// The graph is undirected and connected: if a is in b->neighbors, then b is in a->neighbors.
// There are no self-loops and no repeated edges. The order of neighbors matters.
// Return a deep copy of the graph reachable from `node`: new nodes with the same values
// and the same neighbor order, sharing nothing with the original. nullptr -> nullptr.
// The nodes of the copy are created with `new`; the caller deletes them.
Node* clone_graph(Node* node) {
	if (node == nullptr) { return nullptr; }

	std::unordered_map<Node*, Node*> viewed;
	viewed[node] = new Node{node->val};
	std::queue<Node*> q;
	q.push(node);

	while (!q.empty()) {
		Node* current{q.front()};
		q.pop();

		for (Node* neighbor: current->neighbors) {
			if (!viewed.contains(neighbor)) {
				viewed[neighbor] = new Node(neighbor->val);
				q.push(neighbor);
			}

			viewed[current]->neighbors.push_back(viewed[neighbor]);
		}
	}

	return viewed[node];
}

// Builds a graph from an adjacency list (0-based indices); node i has the value i + 1.
std::vector<Node> build_graph(const std::vector<std::vector<int>>& adjacency) {
    std::vector<Node> nodes(adjacency.size());
    for (std::size_t i = 0; i < nodes.size(); ++i) nodes[i].val = static_cast<int>(i) + 1;
    for (std::size_t i = 0; i < nodes.size(); ++i)
        for (int j : adjacency[i]) nodes[i].neighbors.push_back(&nodes[static_cast<std::size_t>(j)]);
    return nodes;
}

void report(const std::string& label, bool ok) {
    std::cout << label << ": " << (ok ? "OK" : "FAIL") << '\n';
}

// Walks the original and the copy together. Checks that every original node has exactly one copy,
// the copy is a new node with the same value and the same neighbors in the same order,
// and that no node of the copy is a node of the original.
// Collects all distinct nodes of the copy in `copies` so that the caller can delete them.
bool same_graph(Node* original, Node* copy, std::vector<Node*>& copies) {
    if (original == nullptr || copy == nullptr) return original == copy;
    std::unordered_set<Node*> originals;
    {
        std::unordered_set<Node*> seen{original};
        std::vector<Node*> q{original};
        for (std::size_t head = 0; head < q.size(); ++head)
            for (Node* next : q[head]->neighbors)
                if (seen.insert(next).second) q.push_back(next);
        originals = seen;
    }
    if (originals.count(copy) != 0) return false;
    std::unordered_map<Node*, Node*> to_copy;
    std::unordered_set<Node*> used_copies;
    std::vector<Node*> queue{original};
    to_copy[original] = copy;
    used_copies.insert(copy);
    copies.push_back(copy);
    bool ok = true;
    for (std::size_t head = 0; head < queue.size(); ++head) {
        Node* o = queue[head];
        Node* c = to_copy[o];
        if (c->val != o->val || c->neighbors.size() != o->neighbors.size()) { ok = false; continue; }
        for (std::size_t i = 0; i < o->neighbors.size(); ++i) {
            Node* on = o->neighbors[i];
            Node* cn = c->neighbors[i];
            if (cn == nullptr || originals.count(cn) != 0) { ok = false; continue; }
            const auto it = to_copy.find(on);
            if (it == to_copy.end()) {
                if (!used_copies.insert(cn).second) { ok = false; continue; }
                to_copy[on] = cn;
                copies.push_back(cn);
                queue.push_back(on);
            } else if (it->second != cn) {
                ok = false;
            }
        }
    }
    return ok;
}

void delete_copies(const std::vector<Node*>& copies) {
    for (Node* node : copies) delete node;
}

// Snapshot of the original: values and neighbor pointers.
bool unchanged(const std::vector<Node>& nodes, const std::vector<std::vector<int>>& adjacency) {
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i].val != static_cast<int>(i) + 1) return false;
        if (nodes[i].neighbors.size() != adjacency[i].size()) return false;
        for (std::size_t k = 0; k < adjacency[i].size(); ++k)
            if (nodes[i].neighbors[k] != &nodes[static_cast<std::size_t>(adjacency[i][k])]) return false;
    }
    return true;
}

void test_clone(const std::string& label, const std::vector<std::vector<int>>& adjacency) {
    std::vector<Node> nodes = build_graph(adjacency);
    Node* root = nodes.empty() ? nullptr : &nodes[0];
    Node* copy = clone_graph(root);
    std::vector<Node*> copies;
    const bool ok = same_graph(root, copy, copies) && unchanged(nodes, adjacency);
    delete_copies(copies);
    report(label, ok);
}

void test_big(const std::string& label, const std::vector<std::vector<int>>& adjacency) {
    std::vector<Node> nodes = build_graph(adjacency);
    const auto start = std::chrono::steady_clock::now();
    Node* copy = clone_graph(&nodes[0]);
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const bool fast = elapsed < std::chrono::seconds(1);
    std::vector<Node*> copies;
    const bool ok = same_graph(&nodes[0], copy, copies) && copies.size() == nodes.size();
    delete_copies(copies);
    report(label + (fast ? "" : " (too slow)"), ok && fast);
}

std::vector<std::vector<int>> path_graph(int n) {
    std::vector<std::vector<int>> adj(static_cast<std::size_t>(n));
    for (int i = 0; i + 1 < n; ++i) {
        adj[static_cast<std::size_t>(i)].push_back(i + 1);
        adj[static_cast<std::size_t>(i + 1)].push_back(i);
    }
    return adj;
}

void test_all() {
    test_clone("null graph", {});
    test_clone("one node, no neighbors", {{}});
    test_clone("two nodes", {{1}, {0}});
    test_clone("description example: square",
        {{1, 3}, {0, 2}, {1, 3}, {0, 2}});
    test_clone("triangle",
        {{1, 2}, {0, 2}, {0, 1}});
    test_clone("path of 5",
        {{1}, {0, 2}, {1, 3}, {2, 4}, {3}});
    test_clone("star",
        {{1, 2, 3, 4}, {0}, {0}, {0}, {0}});
    test_clone("star, start from a leaf",
        {{1}, {0, 2, 3, 4}, {1}, {1}, {1}});
    test_clone("complete graph of 5",
        {{1, 2, 3, 4}, {0, 2, 3, 4}, {0, 1, 3, 4}, {0, 1, 2, 4}, {0, 1, 2, 3}});
    test_clone("neighbor order differs from node order",
        {{3, 1, 2}, {2, 0}, {1, 0, 3}, {2, 0}});
    test_clone("two cycles sharing a node",
        {{1, 2, 3, 4}, {0, 2}, {0, 1}, {0, 4}, {0, 3}});
    test_clone("binary tree of 7",
        {{1, 2}, {0, 3, 4}, {0, 5, 6}, {1}, {1}, {2}, {2}});
    test_clone("ring of 6",
        {{1, 5}, {0, 2}, {1, 3}, {2, 4}, {3, 5}, {4, 0}});
    test_clone("random 1: 8 nodes",
        {{3, 5}, {2, 4, 7}, {1, 6}, {0, 7}, {1, 6}, {0}, {2, 4}, {1, 3}});
    test_clone("random 2: 12 nodes",
        {{1, 4, 9}, {0, 2, 7}, {1, 3, 10}, {2, 5}, {0, 6, 11}, {3, 8}, {4, 7}, {1, 6, 9}, {5, 10}, {0, 7}, {2, 8, 11}, {4, 10}});

    // ---- big tests ----
    test_big("big: path of 200000 nodes", path_graph(200000));
    {
        std::vector<std::vector<int>> adj(200000);
        for (int i = 0; i < 200000; ++i) {
            adj[static_cast<std::size_t>(i)].push_back((i + 1) % 200000);
            adj[static_cast<std::size_t>(i)].push_back((i + 199999) % 200000);
        }
        test_big("big: ring of 200000 nodes", adj);
    }
    {
        std::vector<std::vector<int>> adj(200000);
        for (int i = 1; i < 200000; ++i) {
            adj[0].push_back(i);
            adj[static_cast<std::size_t>(i)].push_back(0);
        }
        test_big("big: star with 199999 leaves", adj);
    }
    {
        // random connected graph: a random tree plus extra edges, no duplicates
        const int n = 100000;
        std::vector<std::vector<int>> adj(static_cast<std::size_t>(n));
        std::unordered_set<long long> edges;
        const auto add = [&](int a, int b) {
            if (a == b) return;
            const long long key = static_cast<long long>(std::min(a, b)) * n + std::max(a, b);
            if (!edges.insert(key).second) return;
            adj[static_cast<std::size_t>(a)].push_back(b);
            adj[static_cast<std::size_t>(b)].push_back(a);
        };
        unsigned x = 8675309u;
        for (int i = 1; i < n; ++i) {
            x = x * 1103515245u + 12345u;
            add(i, static_cast<int>((x >> 8) % static_cast<unsigned>(i)));
        }
        for (int k = 0; k < 200000; ++k) {
            x = x * 1103515245u + 12345u;
            const int a = static_cast<int>((x >> 8) % static_cast<unsigned>(n));
            x = x * 1103515245u + 12345u;
            const int b = static_cast<int>((x >> 8) % static_cast<unsigned>(n));
            add(a, b);
        }
        test_big("big: random connected graph, 100000 nodes, about 300000 edges", adj);
    }
}

}  // namespace

int main() {
    test_all();
    return 0;
}