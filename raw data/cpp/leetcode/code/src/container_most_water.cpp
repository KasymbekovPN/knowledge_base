#include "container_most_water.h"

#include <algorithm>
#include <iostream>
#include <format>

namespace container_most_water {

int max_area(const std::vector<int>& heights) {
    int left{};
    int right{static_cast<int>(heights.size()) - 1};
    int best{};

    while (left < right) {
        int width = right - left;
        int area{std::min(heights[left], heights[right]) * width};
        best = std::max(best, area);

        if (heights[left] < heights[right]) {
            ++left;
        } else {
            --right;
        }
    }

    return best;
}

void demo() {
    const std::vector<int> heights = {1,8,6,2,5,4,8,3,7};
    std::cout << std::format("max area {}\n", max_area(heights));
}

}