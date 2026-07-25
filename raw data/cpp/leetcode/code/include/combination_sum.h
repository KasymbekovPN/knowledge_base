#pragma once

#include <vector>

namespace combination_sum {
    void backtrace(const std::vector<int>&, const int, const int, std::vector<int>&, std::vector<std::vector<int>>&);
    std::vector<std::vector<int>> combination_sum(const std::vector<int>&, const int);
    void demo();
}