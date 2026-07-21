#include "best_time_buy_sell.h"

#include <vector>
#include <iostream>
#include <format>
#include <algorithm>
#include <limits>

namespace best_time_buy_sell {

int max_profit(const std::vector<int> &prices) {
    int min_price{std::numeric_limits<int>::max()};
    int best_price{0};

    for (int price: prices) {
        min_price = std::min(min_price, price);
        best_price = std::max(best_price, price - min_price);
    }

    return best_price;
}

void demo() {
    const std::vector<int> PRICES = {10, 2, 3, 4, 5, 6, 3, 7, 18, 9};
    std::cout << std::format("best_time_buy_sell: {}\n", max_profit(PRICES));
}

}
