#include "topological_sort.h"

#include <vector>
#include <queue>
#include <iostream>
#include <format>

namespace topological_sort {
bool can_finish_kahn(const int num_courses, const std::vector<std::vector<int>>& prerequisites) {
    std::vector<std::vector<int>> adj(num_courses);
    std::vector<int> in_degree(num_courses, 0);

    for (const auto& p : prerequisites ) {
        const int course{p[0]};
        const int prereq{p[1]};
        adj[prereq].push_back(course);
        ++in_degree[course];
    }

    std::queue<int> q;
    for (int i{}; i < num_courses; ++i) {
        if (in_degree[i] == 0) {
            // курсы без предпосылок можно взять сразу
            q.push(i);
        }
    }

    int processed{};
    while (!q.empty()) {
        const int current{q.front()};
        q.pop();
        ++processed;

        for (const int next: adj[current]) {
            if (--in_degree[next] == 0) {
                // все предпосылки next пройдены -> можно брать
                q.push(next);
            }
        }
    }

    return processed == num_courses;
}

bool has_cycle(const int node, std::vector<std::vector<int>>& adj, std::vector<Color>& colors) {
    colors[node] = Color::Gray;

    for (const int next: adj[node]) {
        if (colors[next] == Color::Gray) {
            // обратное ребро -> цикл найден
            return true;
        }
        if (colors[next] == Color::White && has_cycle(next, adj, colors)) {
            return true;
        }
    }

    // узел полностью обработан, циклов через него нет
    colors[node] = Color::Black;

    return false;
}

bool can_finish_dfs(const int num_courses, const std::vector<std::vector<int>>& prerequisites) {
    std::vector<std::vector<int>> adj(num_courses);
    for (const auto& p : prerequisites ) {
        // p[1] -> p[0] (prereq -> course)
        adj[p[1]].push_back(p[0]);
    }

    std::vector<Color> colors(num_courses, Color::White);
    for (int i{}; i < num_courses; ++i) {
        if (colors[i] == Color::White && has_cycle(i, adj, colors)) {
            // цикл найден -> расписание невозможно
            return true;
        }
    }

    return true;
}

void demo() {
    constexpr int NUM_COURSES{4};
    const std::vector<std::vector<int>> PREREQUISITES{
        {1, 0},
        {2, 0},
        {3, 1},
        {3, 2}
    };

    std::cout << std::format("KAHN: {}\n", can_finish_kahn(NUM_COURSES, PREREQUISITES));
    std::cout << std::format("DFS: {}\n", can_finish_dfs(NUM_COURSES, PREREQUISITES));
}

}
