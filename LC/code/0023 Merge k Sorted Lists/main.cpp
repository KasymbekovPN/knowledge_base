#include <format>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

namespace {

    struct Node {
        int val;
        Node* next;
        explicit Node(const int x) : val(x), next(nullptr) {}
    };

    Node* create_nodes(std::vector<int>& nums) {
        if (nums.empty()) return nullptr;

        const auto value = nums.front();
        nums.erase(nums.begin(), nums.begin() + 1);

        const auto node = new Node(value);
        node->next = create_nodes(nums);

        return node;
    }

    void display_lists(const Node* head) {
        if (head == nullptr) {
            std::cout << "NULL\n";
            return;
        }

        std::cout << std::format("{} ", head->val);
        display_lists(head->next);
    }

    Node* merge_k_lists(const std::vector<Node*>& lists) {
        auto cmp = [](const Node* a, const Node* b) -> bool { return a->val > b->val; };
        std::priority_queue<Node*, std::vector<Node*>, decltype(cmp)> heap(cmp);

        for (const auto n: lists) {
            if (n) heap.push(n);
        }

        Node dummy{0};
        Node* tail = &dummy;

        while (!heap.empty()) {
            Node* curr = heap.top();
            heap.pop();

            tail->next = curr;
            tail = tail->next;

            if (curr->next) heap.push(curr->next);
        }

        return dummy.next;
    }

}

int main() {
    std::vector<int> nums0 = {1, 2, 3};
    std::vector<int> nums1 = {3, 4, 5};
    std::vector<int> nums2 = {5, 6, 7};

    const auto n0{create_nodes(nums0)};
    const auto n1{create_nodes(nums1)};
    const auto n2{create_nodes(nums2)};

    std::cout << "N0\n";
    display_lists(n0);
    std::cout << "N1\n";
    display_lists(n1);
    std::cout << "N2\n";
    display_lists(n2);

    const std::vector<Node*> nodes{n0, n1, n2};
    const auto result{merge_k_lists(nodes)};
    display_lists(result);
}
