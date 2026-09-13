#include <algorithm>
#include <iostream>
#include <format>
#include <queue>
#include <vector>

namespace {

    int find_kth_largest(const std::vector<int>& nums, const int k) {
        std::priority_queue<int, std::vector<int>, std::greater<>> min_on_top_heap;

        for (int num: nums) {
            min_on_top_heap.push(num);
            if (min_on_top_heap.size() > k) {
                min_on_top_heap.pop();
            }
        }

        return min_on_top_heap.top();
    }

    template<typename T>
    void display_vector(const std::vector<T>& vec, const std::string& label) {
        std::cout << std::format("[{}] (", label);
        for (const auto& e : vec) {
            std::cout << e << " ";
        }
        std::cout << ")\n";
    }

}

int main() {
    const std::vector<int> nums = {1, 10, 23, 66, 1, 4, 5};
    std::vector<int> sorted = nums;
    std::ranges::sort(sorted);

    display_vector(nums, "nums");
    display_vector(sorted, "sorted");

    std::cout << find_kth_largest(nums, 4) << std::endl;
}
