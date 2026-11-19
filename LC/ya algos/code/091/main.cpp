#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include <chrono>

namespace {

struct ListNode {
    int val;
    ListNode* next;
};

// Return the node where the cycle begins, or nullptr if the list has no cycle.
// The list must not be modified.
ListNode* detect_cycle(ListNode* head) {
    const ListNode* slow{head};
    const ListNode* fast{head};
    
    while (fast != nullptr && fast->next != nullptr) {
	    slow = slow->next;
	    fast = fast->next->next;
	    
	    if (slow == fast) {
		    ListNode* start{head};
		    while (start != slow) {
			    slow = slow->next;
			    start = start->next;
		    }
		    
		    return start;
	    }
    }
    
    return nullptr;
}

// Builds a list of n nodes. cycle_pos >= 0: the last node points to nodes[cycle_pos]; -1: no cycle.
std::vector<ListNode> build_list(std::size_t n, long long cycle_pos) {
    std::vector<ListNode> nodes(n);
    for (std::size_t i = 0; i < n; ++i) {
        nodes[i].val = static_cast<int>(i);
        nodes[i].next = (i + 1 < n) ? &nodes[i + 1] : nullptr;
    }
    if (n > 0 && cycle_pos >= 0) nodes[n - 1].next = &nodes[static_cast<std::size_t>(cycle_pos)];
    return nodes;
}

void report(const std::string& label, bool ok) {
    std::cout << label << ": " << (ok ? "OK" : "FAIL") << '\n';
}

bool same_links(const std::vector<ListNode>& nodes, const std::vector<ListNode*>& saved) {
    for (std::size_t i = 0; i < nodes.size(); ++i)
        if (nodes[i].next != saved[i]) return false;
    return true;
}

void test_cycle(const std::string& label, std::size_t n, long long cycle_pos) {
    std::vector<ListNode> nodes = build_list(n, cycle_pos);
    std::vector<ListNode*> saved;
    for (const auto& node : nodes) saved.push_back(node.next);
    ListNode* head = nodes.empty() ? nullptr : &nodes[0];
    ListNode* expected = cycle_pos >= 0 ? &nodes[static_cast<std::size_t>(cycle_pos)] : nullptr;
    const bool ok = detect_cycle(head) == expected && same_links(nodes, saved);
    report(label, ok);
}

void test_big(const std::string& label, std::size_t n, long long cycle_pos) {
    std::vector<ListNode> nodes = build_list(n, cycle_pos);
    ListNode* expected = cycle_pos >= 0 ? &nodes[static_cast<std::size_t>(cycle_pos)] : nullptr;
    const auto start = std::chrono::steady_clock::now();
    ListNode* got = detect_cycle(&nodes[0]);
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const bool fast = elapsed < std::chrono::seconds(1);
    report(label + (fast ? "" : " (too slow)"), got == expected && fast);
}

void test_all() {
    test_cycle("empty list", 0, -1);
    test_cycle("one node, no cycle", 1, -1);
    test_cycle("one node pointing to itself", 1, 0);
    test_cycle("two nodes, no cycle", 2, -1);
    test_cycle("two nodes, cycle at head", 2, 0);
    test_cycle("two nodes, cycle at the last node", 2, 1);
    test_cycle("three nodes, cycle at head", 3, 0);
    test_cycle("three nodes, cycle at the middle", 3, 1);
    test_cycle("three nodes, last node points to itself", 3, 2);
    test_cycle("five nodes, no cycle", 5, -1);
    test_cycle("five nodes, cycle at head", 5, 0);
    test_cycle("five nodes, cycle at 2", 5, 2);
    test_cycle("six nodes, cycle at 3 (even length)", 6, 3);
    test_cycle("seven nodes, cycle at 1", 7, 1);
    test_cycle("tail 10, cycle of length 3", 13, 10);
    test_cycle("tail 3, cycle of length 10", 13, 3);
    test_cycle("tail 100, cycle of length 7", 107, 100);
    test_cycle("tail 7, cycle of length 100", 107, 7);
    test_cycle("100 nodes, no cycle", 100, -1);

    // ---- big tests ----
    test_big("big: 1000000 nodes, no cycle", 1000000, -1);
    test_big("big: 1000000 nodes, cycle at head", 1000000, 0);
    test_big("big: 1000000 nodes, cycle in the middle", 1000000, 500000);
    test_big("big: 1000000 nodes, last node points to itself", 1000000, 999999);
    test_big("big: 1000000 nodes, cycle at 3", 1000000, 3);
}

}  // namespace

int main() {
    test_all();
    return 0;
}