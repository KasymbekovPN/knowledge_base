#include "knapsack01.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <format>

namespace knapsack01 {

int knapsack(const int capacity, const std::vector<int>& weights, const std::vector<int>& values) {
    const int N{static_cast<int>(weights.size())};
    std::vector<std::vector<int>> dp(N+1, std::vector<int>(capacity+1, 0));
    const auto print = [&dp](const int i, const int j) {
        std::cout << std::format("{} {}\n", i, j);
        for (const auto& vec: dp) {
            for (const auto& item: vec) {
                std::cout << std::format("{} ", item);
            }
            std::cout << '\n';
        }
        std::cout << "\n\n";
    };

    for (int i{1}; i<=N; ++i) {
        for (int w{0}; w<=capacity; ++w) {
            // вариант "не берём предмет i"
            dp[i][w] = dp[i-1][w];

            if (weights[i-1] <= w) {
                dp[i][w] = std::max(
                    dp[i][w],
                    dp[i-1][w - weights[i-1]] + values[i-1]);
            }
            // print(i, w);
        }
    }

    return dp[N][capacity];
}

int knapsack_1d(const int capacity, const std::vector<int>& weights, const std::vector<int>& values) {
    const int N{static_cast<int>(weights.size())};
    std::vector<int> dp(capacity+1, 0);

    for (int i{}; i < N; ++i) {
        for (int w{capacity}; w >= weights[i]; --w) {
            dp[w] = std::max(dp[w], dp[w - weights[i]] + values[i]);
        }
    }

    return dp[capacity];
}

void demo() {
    constexpr int CAPACITY{7};
    const std::vector<int> WEIGHTS{1, 3, 4, 5};
    const std::vector<int> VALUES{1, 4, 5, 7};

    std::cout << std::format("knapsack: {}\n", knapsack(CAPACITY, WEIGHTS, VALUES));
    std::cout << std::format("knapsack_1d: {}\n", knapsack_1d(CAPACITY, WEIGHTS, VALUES));
}

}