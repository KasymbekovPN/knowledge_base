#include "subsets.h"

#include <string>
#include <vector>
#include <iostream>
#include <format>

namespace subsets {
// index start solution
void backtrace(const std::vector<int>& nums,
                const int start,
                std::vector<int>& path,
                std::vector<std::vector<int>>& result) {
    // текущий path — уже готовое подмножество, сохраняем сразу
    result.push_back(path);

    for (int i{start}; i < static_cast<int>(nums.size()); ++i) {
        path.push_back(nums[i]);
        // следующий элемент выбираем только из "правее i"
        backtrace(nums, i + 1, path, result);
        path.pop_back();
    }
}
std::vector<std::vector<int>> subsets(const std::vector<int>& nums) {
    std::vector<std::vector<int>> result;
    std::vector<int> path;
    backtrace(nums, 0, path, result);

    return result;
}

// bitmask solution
std::vector<std::vector<int>> subsets_bitmask(const std::vector<int>& nums) {
    const int N{static_cast<int>(nums.size())};
    std::vector<std::vector<int>> result;

    for (int mask{}; mask < (1 << N); ++mask) {
        std::vector<int> subset;
        for (int j{}; j < N; ++j) {
            if (mask & (1 << j)) {
                subset.push_back(nums[j]);
            }
        }
        result.push_back(std::move(subset));
    }

    return result;
}

// iterative solution
std::vector<std::vector<int>> subsets_iterative(const std::vector<int>& nums) {
    // начинаем с пустого подмножества
    std::vector<std::vector<int>> result{{}};

    for (const auto& num : nums) {
        const int SIZE{static_cast<int>(result.size())};
        for (int i{}; i < SIZE; ++i) {
            std::vector<int> new_subset = result[i];
            new_subset.push_back(num);
            result.push_back(std::move(new_subset));
        }
    }

    return result;
}

static void print(const std::vector<std::vector<int>>& nums, const std::string& prefix) {
    std::cout << std::format("{}: ", prefix);
    for (const auto& vec : nums) {
        for (const auto& num : vec) {
            std::cout << std::format("{} ", num);
        }
        std::cout << '\n';
    }
    std::cout << "\n\n";
}

void demo() {
    const std::vector<int> NUMS{1, 2, 3};

    const auto result0 = subsets(NUMS);
    print(result0, "START");

    const auto result1 = subsets_bitmask(NUMS);
    print(result1, "MASK");

    const auto result2 = subsets_iterative(NUMS);
    print(result2, "ITERATIVE");
}
}