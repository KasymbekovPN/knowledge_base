#include "merge_intervals.h"

#include <algorithm>
#include <vector>
#include <iostream>
#include <format>

namespace merge_intervals {

std::vector<std::vector<int>> merge(std::vector<std::vector<int>>& intervals) {
    if (intervals.empty()) return {};

    std::ranges::sort(
        intervals,
        [](const auto& a, const auto& b) {
            return a[0] < b[0];
        });

    std::vector<std::vector<int>> result;
    result.push_back(intervals[0]);

    for (int i{1}; i < static_cast<int>(intervals.size()); ++i) {
        auto& last = result.back();
        if (const auto& cur = intervals[i]; cur[0] < last[1]) {
            last[1] = std::max(last[1], cur[1]);
        } else {
            result.push_back(cur);
        }
    }

    return result;
}

void demo() {
    auto INTERVALS = std::vector<std::vector<int> >{
        {1, 3}, {15, 18}, {2, 6}, {8, 10}
    };
    for (const auto result = merge(INTERVALS); auto i : result) {
        std::cout << std::format("[{}...{}]\n", i[0], i[1]);
    }
}

}
