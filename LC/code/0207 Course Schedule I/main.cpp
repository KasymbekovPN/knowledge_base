#include <iostream>
#include <format>
#include <vector>
#include <queue>

namespace bfs {
    static bool can_finish(const int num_courses, const std::vector<std::vector<int>>& prerequisites) {
        std::vector<std::vector<int>> graph(num_courses);
        std::vector<int> in_degree(num_courses, 0);

        for (auto& p: prerequisites) {
            const int course = p[0];
            const int prereq = p[1];
            graph[prereq].push_back(course);
            in_degree[course]++;
        }

        std::queue<int> q;
        for (int i{}; i < num_courses; ++i) {
            if (in_degree[i] == 0) q.push(i);
        }

        int processed{};
        while (!q.empty()) {
            const int cur{q.front()};
            q.pop();
            processed++;

            for (int next: graph[cur]) {
                if (--in_degree[next] == 0) q.push(next);
            }
        }

        return processed == num_courses;
    }

    static void run_test(const int num_courses,
                         const std::vector<std::vector<int>>& prerequisites,
                         const bool expected) {
        const bool result{can_finish(num_courses, prerequisites) == expected};
        std::cout << std::format("num_courses -> {}, expected: {}, result: {}\n",
            num_courses,
            expected == true ? "true" : "false",
            result ? "OK" : "FAIL");
    }

    static void tests() {
        // Пример 1: numCourses=2, prerequisites=[[1,0]]
        // Чтобы взять курс 1, нужен курс 0. Цикла нет -> можно пройти все.
        run_test(2, {{1, 0}}, true);

        // Пример 2: numCourses=2, prerequisites=[[1,0],[0,1]]
        // Курс 1 требует 0, курс 0 требует 1 -> цикл -> невозможно.
        run_test(2, {{1, 0}, {0, 1}}, false);

        // Пример 3: numCourses=4, prerequisites=[[1,0],[2,1],[3,2]]
        // Линейная цепочка 0->1->2->3, цикла нет.
        run_test(4, {{1, 0}, {2, 1}, {3, 2}}, true);

        // Пример 4: numCourses=3, prerequisites=[[0,1],[1,2],[2,0]]
        // Цикл 0->1->2->0.
        run_test(3, {{0, 1}, {1, 2}, {2, 0}}, false);

        // Пример 5: numCourses=3, prerequisites={} — нет зависимостей вообще
        run_test(3, {}, true);
    }

}

namespace dfs {
    static constexpr int STATE_UNVISITED{0};
    static constexpr int STATE_VISITING{1};
    static constexpr int STATE_VISITED{2};

    static bool dfs(const int node, const std::vector<std::vector<int>>& graph, std::vector<int>& state) {
        if (state[node] == STATE_VISITING) return false; // visiting -> нашли цикл
        if (state[node] == STATE_VISITED) return true; // visited -> уже проверено, цикла нет

        state[node] = STATE_VISITING; // помечаем "сейчас обрабатываем"
        for (const auto& next: graph[node]) {
            if (!dfs(next, graph, state)) return false;
        }

        state[node] = STATE_VISITED; // полностью обработан

        return true;
    }

    static bool can_finish(const int num_courses, const std::vector<std::vector<int>>& prerequisites) {
        std::vector<std::vector<int>> graph(num_courses);
        for (auto& p: prerequisites) {
            graph[p[0]].push_back(p[1]); // graph[a] = список пререквизитов курса a
        }

        std::vector<int> state(num_courses, 0);

        for (int i{}; i < num_courses; ++i) {
            if (!dfs(i, graph, state)) return false;
        }

        return true;
    }

    static void run_test(const int num_courses,
                         const std::vector<std::vector<int>>& prerequisites,
                         const bool expected) {
        const bool result{can_finish(num_courses, prerequisites) == expected};
        std::cout << std::format("num_courses -> {}, expected: {}, result: {}\n",
            num_courses,
            expected == true ? "true" : "false",
            result ? "OK" : "FAIL");
    }

    static void tests() {
        // Пример 1: numCourses=2, prerequisites=[[1,0]]
        // Курс 1 требует курс 0. Цикла нет.
        run_test(2, {{1, 0}}, true);

        // Пример 2: numCourses=2, prerequisites=[[1,0],[0,1]]
        // Взаимный цикл 1<->0.
        run_test(2, {{1, 0}, {0, 1}}, false);

        // Пример 3: numCourses=4, prerequisites=[[1,0],[2,1],[3,2]]
        // Линейная цепочка зависимостей, цикла нет.
        run_test(4, {{1, 0}, {2, 1}, {3, 2}}, true);

        // Пример 4: numCourses=3, prerequisites=[[0,1],[1,2],[2,0]]
        // Цикл 0->1->2->0.
        run_test(3, {{0, 1}, {1, 2}, {2, 0}}, false);

        // Пример 5: без зависимостей вообще
        run_test(3, {}, true);

        // Пример 6: общий пререквизит у нескольких курсов (не цикл, но заходит в один узел дважды)
        // Курсы 1 и 2 оба требуют курс 0. DFS должен корректно пройти через "visited" узел 0
        // без ложного срабатывания на цикл.
        run_test(3, {{1, 0}, {2, 0}}, true);
    }

}


int main() {
    // bfs::tests();
    dfs::tests();

    return 0;
}
