#include "top_k_freq_elements.h"

#include <vector>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <format>

namespace top_k_freq_elements {

// min-heap solution
std::vector<int> top_k_freq(const std::vector<int>& nums, const int k) {
    std::unordered_map<int, int> freq;
    for (const auto& num: nums) ++freq[num];

    // min-heap по частоте: {frequency, value}
    std::priority_queue<
        std::pair<int, int>,
        std::vector<std::pair<int, int>>,
        std::greater<std::pair<int, int>>
    > min_heap;

    for (const auto& [value, count]: freq) {
        min_heap.push({count, value});
        // выбрасываем наименее частый, если размер превысил k
        if (static_cast<int>(min_heap.size()) > k) min_heap.pop();
    }

    std::vector<int> result;
    result.reserve(k);
    while (!min_heap.empty()) {
        result.push_back(min_heap.top().second);
        min_heap.pop();
    }

    return result;
}

// bucket sort solution
std::vector<int> top_k_freq_bucket(const std::vector<int>& nums, const int k) {
    std::unordered_map<int, int> freq;
    for (const auto& num: nums) ++freq[num];

    const int N{static_cast<int>(nums.size())};
    // buckets[f] = значения с частотой f
    std::vector<std::vector<int>> buckets(N + 1);

    for (const auto& [value, count]: freq) {
        buckets[count].push_back(value);
    }

    std::vector<int> result;
    for (int f{N}; f >= 1 && static_cast<int>(result.size()) < k; --f) {
        for (const auto& value: buckets[f]) {
            result.push_back(value);
            if (static_cast<int>(result.size()) == k) break;
        }
    }

    return result;
}

static void print(std::vector<int>&& nums, std::string&& label) {
    std::cout << label << ": ";
    for (const auto& num: nums) std::cout << num << " ";
    std::cout << std::endl;
}

void demo() {
    constexpr int K{2};
    const std::vector<int> NUMS{1,1,1,2,2,3};

    print(top_k_freq(NUMS, K), "min-heap solution");
    print(top_k_freq_bucket(NUMS, K), "bucket sort solution");
}

}