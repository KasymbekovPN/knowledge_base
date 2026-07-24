#pragma once

#include <vector>

namespace subsets {
    // index start solution
    void backtrace(const std::vector<int>&, const int, std::vector<int>&, std::vector<std::vector<int>>&);
    std::vector<std::vector<int>> subsets(const std::vector<int>&);

    // bitmask solution
    std::vector<std::vector<int>> subsets_bitmask(const std::vector<int>&);

    // iterative solution
    std::vector<std::vector<int>> subsets_iterative(const std::vector<int>&);

    void demo();
}