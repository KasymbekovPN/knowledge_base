#pragma once
#include <tuple>
#include <vector>

namespace max_subarray {
    std::tuple<int, int, int> find_max_subarray(const std::vector<int>& sequence);
    void demo();
}
