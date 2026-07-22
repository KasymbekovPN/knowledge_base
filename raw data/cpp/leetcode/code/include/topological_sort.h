#pragma once

#include <vector>

namespace topological_sort {
    bool can_finish_kahn(const int num_courses, const std::vector<std::vector<int>>& prerequisites);

    enum class Color {White, Gray, Black};
    bool has_cycle(const int node, std::vector<std::vector<int>>& adj, std::vector<Color>& colors);
    bool can_finish_dfs(const int num_courses, const std::vector<std::vector<int>>& prerequisites);

    void demo();
}
