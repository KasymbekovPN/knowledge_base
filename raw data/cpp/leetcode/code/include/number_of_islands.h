#pragma once

#include <vector>

namespace number_of_islands {
    void dfs(std::vector<std::vector<char>> &grid, const int r, const int c);
    int num_islands_dfs(std::vector<std::vector<char>> &grid);
    int num_islands_bfs(std::vector<std::vector<char>> &grid);
    void demo();
}