#include <vector>
#include <queue>
#include <iostream>
#include <ranges>

namespace bfs {
    static std::vector<int> find_order(const int num_courses, const std::vector<std::vector<int>> &prerequisites ) {
        std::vector<std::vector<int>> graph(num_courses);
        std::vector<int> in_degree(num_courses, 0);

        for (const auto& p: prerequisites) {
            const int course{p[0]};
            const int prereq{p[1]};
            in_degree[course]++;
        }

        std::queue<int> q;
        for (int i{}; i < num_courses; i++) {
            if (in_degree[i] == 0) q.push(i);
        }

        std::vector<int> order;
        order.reserve(num_courses);

        while (!q.empty()) {
            const int cur{q.front()};
            q.pop();
            order.push_back(cur);

            for (const int next: graph[cur]) {
                if (--in_degree[next] == 0) q.push(next);
            }
        }

        // цикл -> невозможно пройти все курсы
        if (order.size() != num_courses) return {};

        return order;
    }
};

namespace dfs {
    static constexpr int STATE_UNVISITED{0};
    static constexpr int STATE_VISITING{1};
    static constexpr int STATE_VISITED{2};

    static bool dfs(const int node,
                    const std::vector<std::vector<int>> &graph,
                    std::vector<int>& state,
                    std::vector<int>& order) {
        if (state[node] == STATE_VISITING) return true; // цикл
        if (state[node] == STATE_VISITED) return false;

        state[node] = STATE_VISITING;
        for (const auto& next: graph[node]) {
            if (!dfs(next, graph, state, order)) return false;
        }

        state[node] = STATE_VISITED;
        order.push_back(node); // добавляем в момент полного завершения узла

        return true;
    }

    static std::vector<int> find_order(const int num_courses, const std::vector<std::vector<int>> &prerequisites ) {
        std::vector<std::vector<int>> graph(num_courses);
        for (const auto& p: prerequisites) {
            graph[p[0]].push_back(p[1]);
        }

        std::vector<int> state(num_courses, 0);
        std::vector<int> order;

        for (int i{}; i < num_courses; i++) {
            if (!dfs(i, graph, state, order)) return {};
        }

        // критический шаг!
        std::reverse(order.begin(), order.end());

        return order;
    }

}

namespace {
    bool is_valid_order(const int num_courses,
                        const std::vector<std::vector<int>> &prerequisites,
                        const std::vector<int>& order) {
        if (static_cast<int>(order.size()) == num_courses) {
            return order.empty() && num_courses > 0
                ? true
                : order.size() == static_cast<size_t>(num_courses);
        }

        std::vector<int> position(num_courses);
        for (int i{}; i < static_cast<int>(order.size()); i++) position[order[i]] = i;
        for (const auto& p: prerequisites) {
            // prereq должен идти раньше course
            if (position[p[1]] > position[p[0]]) return false;
        }

        return true;
    }

    template <typename T>
    void print_vec(const std::vector<T>& vec, const std::string& label) {
        std::cout << std::format("[{}] (", label);
        std::string delimiter;
        for (int i{}; i < static_cast<int>(vec.size()); i++) {
            std::cout << std::format("{}{}", delimiter, vec[i]);
            delimiter = ", ";
        }
        std::cout << ")\n";
    }

    void run_test(const std::string& name,
                  const int num_courses,
                  const std::vector<std::vector<int>>& prerequisites,
                  const bool expected_possible) {
        const auto prereq_copy_bfs{prerequisites};
        const auto prereq_copy_dfs{prerequisites};

        const auto order_bfs{bfs::find_order(num_courses, prereq_copy_bfs)};
        const auto order_dfs{dfs::find_order(num_courses, prereq_copy_dfs)};

        const bool bfs_valid{
            expected_possible
            ? is_valid_order(num_courses, prerequisites, order_bfs)
            : order_bfs.empty()
        };
        const bool dfs_valid{
            expected_possible
            ? is_valid_order(num_courses, prerequisites, order_dfs)
            : order_dfs.empty()
        };

        std::cout << "[" << name << "] numCourses=" << num_courses << std::endl;
        print_vec<int>(order_bfs, "BFS");
        std::cout << "  -> " << (bfs_valid ? "OK" : "FAIL") << std::endl;
        print_vec<int>(order_dfs, "DFS");
        std::cout << "  -> " << (dfs_valid ? "OK" : "FAIL") << std::endl;
        std::cout << std::endl;
    }
}

int main() {
    // Пример 1: numCourses=2, prerequisites=[[1,0]]
    run_test("Simple record", 2, {{1, 0}}, true);

    // Пример 2: numCourses=4, линейная цепочка
    run_test("Linear chain", 4, {{1, 0}, {2, 1}, {3, 2}}, true);

    // Пример 3: цикл -> невозможно
    run_test("Cycle (impossible)", 2, {{1, 0}, {0, 1}}, false);

    // Пример 4: несколько курсов зависят от общего пререквизита + один зависит от всех
    run_test("Common prerequisite + converging dependency", 4, {{3, 0}, {3, 1}, {3, 2}}, true);

    // Пример 5: без зависимостей вообще (любой порядок валиден)
    run_test("Without dependencies", 3, {}, true);

    // Пример 6: diamond-подобная структура
    run_test("Diamond-structure", 6, {
        {1, 0},
        {2, 0},
        {3, 1},
        {3, 2},
        {4, 3},
        {5, 4}
    }, true);

    // Пример 7: цикл среди трёх узлов
    run_test("Three nodes cycle", 3, {{0, 1}, {1, 2}, {2, 0}}, false);

    return 0;
}
