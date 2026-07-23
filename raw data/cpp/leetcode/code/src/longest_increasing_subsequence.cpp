#include "longest_increasing_subsequence.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <format>

namespace longest_increasing_subsequence {

int len_of_lis(const std::vector<int>& nums) {
    std::cout << "len_of_lis\n";
    const int N{static_cast<int>(nums.size())};
    if (N == 0) return 0;

    // каждый элемент сам по себе -> подпоследовательность длины 1
    std::vector<int> dp(N, 1);
    int best{1};

    const auto print = [&nums, &dp, &best](const int i, const int j) {
        std::cout << "###\nnums: ";
        for (const auto& item : nums) std::cout << std::format("{} ", item);
        std::cout << "\ndp: ";
        for (const auto& item : dp) std::cout << std::format("{} ", item);
        std::cout << std::format("best: {}, i: {}, j: {}\n", best, i, j);
    };

    for (int i{1}; i < N; ++i) {
        for (int j{0}; j < i; ++j) {
            if (nums[j] > nums[i]) {
                dp[i] = std::max(dp[i], dp[j] + 1);
            }
            // print(i, j);
        }
        best = std::max(best, dp[i]);
    }

    return best;
}

int len_of_lis_fast(const std::vector<int>& nums) {
    std::cout << "len_of_lis_fast\n";
    std::vector<int> tails;

    const auto print = [&tails]() {
        for (const auto& item: tails) std::cout << std::format("{} ", item);
        std::cout << "\n";
    };

    for (const auto& item: nums) {
        if (auto it{std::ranges::lower_bound(tails, item)};
            it == tails.end()) {

            tails.push_back(item);
        }
        else {
            *it = item;
        }
        // print();
    }

    return static_cast<int>(tails.size());
}

void demo() {
    const std::vector<int> NUMS{10, 9, 2, 5, 3, 7, 101, 18};
    std::cout << std::format("len_of_lis: {}\n", len_of_lis(NUMS));
    std::cout << std::format("len_of_lis_fast: {}\n", len_of_lis_fast(NUMS));
}

}
