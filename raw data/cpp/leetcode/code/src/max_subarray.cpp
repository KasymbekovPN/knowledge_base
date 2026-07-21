#include "max_subarray.h"

#include <tuple>
#include <vector>
#include <iostream>
#include <format>

namespace max_subarray {

std::tuple<int, int, int> find_max_subarray(const std::vector<int>& sequence) {
    int cur_sum{sequence[0]};
    int best_sum{sequence[0]};
    int start{0};
    int best_start{0};
    int best_end{0};

    for (int i{1}; i < static_cast<int>(sequence.size()); ++i) {
        if (cur_sum < 0) {
            cur_sum = sequence[i];
            start = i;
        } else {
            cur_sum += sequence[i];
        }

        if (cur_sum > best_sum) {
            best_sum = cur_sum;
            best_start = start;
            best_end = i;
        }
    }

    return {best_sum, best_start, best_end};
}

void demo() {
    const std::vector<int> SEQUENCE{1, 2, 3, 4, -1, 5, 6, 7, 8, -10, -20, 9, 7};
    auto [sum, begin, end] = find_max_subarray(SEQUENCE);
    std::cout << std::format("[{}, {}] => {}\n",begin, end, sum);
}

}
