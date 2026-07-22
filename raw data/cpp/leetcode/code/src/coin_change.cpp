#include "coin_change.h"

#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>
#include <format>

namespace coin_change {

int coin_change(const std::vector<int>& coins, const int amount) {
    constexpr int INF{std::numeric_limits<int>::max() / 2};
    std::vector<int> dp(amount + 1, INF);
    dp[0] = 0;

    for (int i{}; i <= amount; ++i) {
        for (const int coin: coins) {
            if (coin <= i && dp[i-coin] != INF) {
                dp[i] = std::min(dp[i], dp[i-coin] + 1);
            }
        }
    }

    return dp[amount] == INF ? -1 : dp[amount];
}

void demo() {
    constexpr int AMOUNT{11};
    const std::vector<int> COINS{1, 2, 5};

    std::cout << std::format("quantity: {}\n", coin_change(COINS, AMOUNT));
}

}
