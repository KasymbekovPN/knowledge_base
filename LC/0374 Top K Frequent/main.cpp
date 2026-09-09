#include <iostream>
#include <format>
#include <queue>
#include <vector>
#include <unordered_map>

namespace {
    template<typename T>
    void display_vector(const std::vector<T>& vec, const std::string& label) {
        std::cout << std::format("[{}] (", label);
        for (const auto& e : vec) {
            std::cout << e << " ";
        }
        std::cout << ")\n";
    }

    std::vector<int> top_k_freq(const std::vector<int>& nums, const int k) {
        std::unordered_map<int, int> freq;
        for (const int n: nums) freq[n]++;

        auto cmp = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.second > b.second; // greater => min-heap
        };
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, decltype(cmp)> heap(cmp);

        for (auto& [num, count]: freq) {
            heap.emplace(num, count);
            if (heap.size() > k) heap.pop();
        }

        std::vector<int> result;
        while (!heap.empty()) {
            result.push_back(heap.top().first);
            heap.pop();
        }

        return result;
    }
}

int main() {
    const std::vector<int> nums = {1, 2, 3, 4, 5, 5, 4, 3, 1, 1, 2, 1, 5};
    const std::vector<int> result = top_k_freq(nums, 5);
    display_vector(result, "result");
}
