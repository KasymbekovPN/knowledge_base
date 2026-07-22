#include "climbing_stairs.h"

#include <vector>
#include <iostream>
#include <format>

namespace climbing_stairs {

int climb_stairs_naive(const int n) {
    if (n <= 2) return n;
    return climb_stairs_naive(n - 1) + climb_stairs_naive(n - 2);
}

int climb_stairs(const int n) {
    if (n <= 2) return n;

    int prev2{1};
    int prev1{2};

    for (int i{3}; i <= n; ++i) {
        const int current{prev1 + prev2};
        prev2 = prev1;
        prev1 = current;
    }

    return prev1;
}

int climb_stairs_dp(const int n) {
    if (n <= 2) return n;

    std::vector<int> dp(n + 1);
    dp[1] = 1;
    dp[2] = 2;

    for (int i{3}; i <= n; ++i) {
        dp[i] = dp[i - 1] + dp[i - 2];
    }

    return dp[n];
}

void demo() {
    std::cout << std::format("climb_stairs_naive: {}\n", climb_stairs_naive(7));
    std::cout << std::format("climb_stairs: {}\n", climb_stairs(7));
    std::cout << std::format("climb_stairs_dp: {}\n", climb_stairs_dp(7));
}

}