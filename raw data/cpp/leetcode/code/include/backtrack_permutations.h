#pragma once

#include <vector>

namespace backtrack_permutations {
    void backtrack(std::vector<int>&, std::vector<int>&, std::vector<bool>&, std::vector<std::vector<int>>&);
    std::vector<std::vector<int>> permute(std::vector<int>&);
    void backtrack_swap(std::vector<int>&, const int, std::vector<std::vector<int>>&);
    void demo();
}