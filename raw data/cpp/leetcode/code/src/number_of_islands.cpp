#include "number_of_islands.h"

#include <vector>
#include <string>
#include <iostream>
#include <format>
#include <queue>
#include <utility>

namespace number_of_islands {

void dfs(std::vector<std::vector<char>> &grid, const int r, const int c) {
    const int rows{static_cast<int>(grid.size())};

    if (const int cols{static_cast<int>(grid[0].size())};
        r < 0 || r >= rows || c < 0 || c >= cols || grid[r][c] != '1') {
        // за границей сетки или не суша (вода/уже посещено)
        return;
    }

    // помечаем как посещённое, "топим" клетку
    grid[r][c] = '0';

    dfs(grid, r + 1, c);
    dfs(grid, r - 1, c);
    dfs(grid, r, c + 1);
    dfs(grid, r, c - 1);
}

int num_islands_dfs(std::vector<std::vector<char>> &grid) {
    if (grid.empty() || grid[0].empty()) return 0;

    const int rows{static_cast<int>(grid.size())};
    const int cols{static_cast<int>(grid[0].size())};
    int count{};

    for (int r{}; r < rows; ++r) {
        for (int c{}; c < cols; ++c) {
           if (grid[r][c] == '1') {
               ++count;
               dfs(grid, r, c);
           }
        }
    }

    return count;
}

int num_islands_bfs(std::vector<std::vector<char>> &grid) {
    if (grid.empty() || grid[0].empty()) return 0;

    const int rows{static_cast<int>(grid.size())};
    const int cols{static_cast<int>(grid[0].size())};
    int count{};

    const int dr[] = {1, -1, 0, 0};
    const int dc[] = {0, 0, 1, -1};

    for (int r{}; r < rows; ++r) {
        for (int c{}; c < cols; ++c) {
            if (grid[r][c] != '1') continue;

            ++count;
            grid[r][c] = '0';
            std::queue<std::pair<int, int>> q;
            q.push({r, c});

            while (!q.empty()) {
                auto [cur_r, cur_c] = q.front();
                q.pop();

                for (int dir{}; dir < 4; ++dir) {
                    const int nr{cur_r + dr[dir]};
                    if (const int nc{cur_c + dc[dir]};
                        nr >= 0 && nr < rows && nc >= 0 && nc < cols && grid[nr][nc] == '1') {
                        // помечаем сразу при добавлении в очередь
                        grid[nr][nc] = '0';
                        q.push({nr, nc});
                    }
                }
            }
        }
    }

    return count;
}

void demo() {
    std::vector<std::vector<char>> grid_dfs = {
        {'1','1','0','0','0'},
        {'1','1','0','0','0'},
        {'0','0','1','0','0'},
        {'0','0','0','1','1'}
    };
    std::cout << std::format("number of islands DFS: {}\n", num_islands_dfs(grid_dfs));

    std::vector<std::vector<char>> grid_bfs = {
        {'1','1','0','0','0'},
        {'1','1','0','0','0'},
        {'0','0','1','0','0'},
        {'0','0','0','1','1'}
    };
    std::cout << std::format("number of islands BFS: {}\n", num_islands_bfs(grid_bfs));
}

}