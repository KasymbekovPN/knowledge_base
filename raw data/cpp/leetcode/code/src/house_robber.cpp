#include "house_robber.h"

#include <vector>
#include <algorithm>
#include <format>
#include <iostream>

namespace house_robber {

int rob(const std::vector<int>& nums) {
    int prev2{0}; // dp[i-2], лучший результат "два дома назад"
    int prev1{0}; // dp[i-1], лучший результат "один дом назад"

    for (const auto& num : nums) {
        const int current{std::max(prev1, prev2 + num)};
        prev2 = prev1;
        prev1 = current;
    }

    return prev1;
}

int rob_dp(const std::vector<int>& nums) {
    const int N{static_cast<int>(nums.size())};
    if (N == 0) return 0;
    if (N == 1) return nums[0];

    std::vector<int> dp(N);
    dp[0] = nums[0];
    dp[1] = std::max(nums[0], nums[1]);

    for (int i{2}; i < N; ++i) {
        dp[i] = std::max(dp[i - 1], dp[i - 2] + nums[i]);
    }

    return dp[N - 1];
}

void demo() {
    const std::vector<int> NUMS{2, 7, 9, 3, 1};
    std::cout << std::format("rob {}\n", rob(NUMS));
    std::cout << std::format("rob_dp {}\n", rob_dp(NUMS));
}

}
