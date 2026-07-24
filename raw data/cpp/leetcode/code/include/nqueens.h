#pragma once

#include <vector>
#include <unordered_set>
#include <string>

namespace nqueens {
    void backtrack(const int row,
                    const int n,
                    std::vector<int>& queens,
                    std::unordered_set<int>& cols,
                    std::unordered_set<int>& diag1,
                    std::unordered_set<int>& diag2,
                    std::vector<std::vector<std::string>>& result);
    std::vector<std::vector<std::string>> solve_nqueens(const int n);
    void backtrack_bitmask(const int row,
                            const int n,
                            const int cols,
                            const int diag1,
                            const int diag2,
                            int& count);
    void demo();
}