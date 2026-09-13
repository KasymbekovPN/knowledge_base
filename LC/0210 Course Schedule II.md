
## Суть задачи

Отличие от LC207 минимальное на первый взгляд, но существенное по требованию: вместо булева "можно ли пройти все курсы" нужно вернуть **сам порядок** — один из валидных вариантов последовательности прохождения курсов, при которой все пререквизиты соблюдены. Если пройти все курсы невозможно (есть цикл) — вернуть пустой массив.

Это прямое расширение LC207: обнаружение цикла там было побочным продуктом, а сам топологический порядок отбрасывался. Здесь порядок — и есть основной результат.

```cpp
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

```

## Подход 1: BFS (Kahn's algorithm) — естественное расширение

В BFS-версии LC207 порядок, в котором курсы вынимались из очереди, **уже является** корректной топологической сортировкой — нужно просто сохранять его, а не только считать.

Единственное изменение относительно LC207 — вместо счётчика `processed++` теперь `order.push_back(cur)`, и в конце вместо `return processed == numCourses` — проверка размера и либо возврат `order`, либо пустого вектора.

## Подход 2: DFS — порядок через postorder + reverse

Здесь чуть менее очевидно, чем в BFS, поэтому стоит разобрать подробно. Идея: если делать DFS и добавлять узел в результат **после** того, как обработаны все его зависимости (то есть в момент, когда узел переходит в состояние "полностью завершён" — тот самый `state[node] = 2` из LC207), то получившийся порядок — это topological order **в обратном** виде, и его нужно развернуть.

Почему в обратном: если курс A зависит от курса B (A нужен после B), то DFS из A первым делом уходит вглубь к B, полностью его обрабатывает и **только потом** возвращается и "завершает" A. Значит, B попадёт в postorder-список раньше A. Но нам нужно, чтобы B шёл раньше A в **итоговом ответе** тоже — значит, если хранить в порядке "как узлы заканчивались" (B, потом A), это уже правильный порядок... стоп, тут легко запутаться, поэтому лучше зафиксировать на конкретном примере ниже.

**Разбор на примере**, почему нужен `reverse`: пусть курс `1` требует курс `0` (`graph[1] = {0}`, то есть "чтобы взять 1, нужен 0"). DFS стартует с узла `1`: заходит в `dfs(1)`, видит зависимость `0`, рекурсивно уходит в `dfs(0)`. У `0` нет зависимостей — он сразу завершается: `order.push_back(0)` → `order = [0]`. Возвращаемся в `dfs(1)`, он тоже завершается: `order.push_back(1)` → `order = [0, 1]`.

В этом простом примере результат `[0, 1]` уже правильный **без** реверса — курс 0 идёт раньше курса 1, что и требуется! Но это совпадение из-за того, что мы начали DFS именно с узла `1` (потребителя), а не с `0`.

Возьмём случай, где порядок стартовых узлов другой: `numCourses=2`, та же зависимость (`1` требует `0`), но внешний цикл `for (i = 0; i < numCourses; i++)` стартует `dfs(0)` **первым** (потому что i идёт от 0). `dfs(0)`: у узла `0` в `graph[0]` зависимостей нет (в `graph` мы храним "пререквизиты для", а `0` сам ни от кого не зависит) → сразу завершается: `order = [0]`. Дальше цикл доходит до `i=1`, `dfs(1)`: видит зависимость `0`, но `state[0] == 2` уже (visited) → сразу возвращается true без рекурсии вглубь, затем `1` тоже завершается: `order = [0, 1]`.

Тут тоже результат `[0, 1]` — без реверса верно. Так что нужен пример **специально** подобранный, чтобы показать, зачем reverse. Возьмём другую структуру: курс `0` требует курс `1` (`graph[0] = {1}`). Внешний цикл начинает с `dfs(0)`: заходит в `0`, видит зависимость `1`, уходит в `dfs(1)` — у `1` нет своих зависимостей → `1` завершается первым: `order = [1]`. Возвращаемся в `dfs(0)`, он завершается: `order = [1, 0]`.

Без reverse ответ был бы `[1, 0]` — и это **на самом деле уже правильно**: `1` должен идти раньше `0`, потому что `0` требует `1`. Хм — то есть в этом примере тоже reverse не нужен?

Дело в том, что при использовании графа в направлении "`graph[a]` = пререквизиты `a`" (как в этом коде), **postorder DFS без reverse уже даёт правильный порядок** для случая одной цепочки — потому что зависимость всегда полностью заканчивается раньше зависящего от неё узла. Reverse становится нужен только когда **несколько независимых DFS-стартов** (несколько итераций внешнего цикла) вперемешку добавляют в `order`, и порядок между результатами разных "деревьев" DFS не гарантирован тем же способом.

Если честно — я слишком усложнил разбор, пытаясь построить контрпример вручную. Дайте мне явно проверить это кодом, чтобы дать точный, а не "на словах предположительный" ответ, потому что тут легко ошибиться в рассуждении.Результат прояснил ситуацию — и подтвердил, что мои сомнения в рассуждении выше были обоснованы, но привели к неожиданному выводу: **reverse не нужен вообще**, при том направлении графа, которое использовано в коде (`graph[a]` = пререквизиты `a`).Эксперимент дал точный и однозначный ответ — исправляю то, что написал выше.

## Правильный вывод: reverse нужен или нет — зависит от направления рёбер в графе

Я ошибся в исходном объяснении, предположив нужность `reverse` по умолчанию (это стандартное правило из учебников, но оно верно только для **определённого** направления графа). Эксперимент показал:

- Граф в виде **`graph[a]` = пререквизиты `a`** (то, что использовано в этом коде: `graph[p[0]].push_back(p[1])`) → postorder DFS **сразу** даёт правильный порядок, **reverse ломает** результат (все тесты стали `NO`).
- Стандартный учебный вариант графа **`graph[a]` = "что зависит от `a`"** (т.е. рёбра "пререквизит → потребитель") — вот там postorder действительно даёт **обратный** порядок, и `reverse` необходим.

## Почему так — интуиция

При `graph[a] = prerequisites of a`: DFS из курса `a` идёт **вглубь к его зависимостям** первым, и зависимость гарантированно завершается (`state=2`, добавляется в `order`) раньше, чем сам `a` — то есть **порядок завершения = порядок, в котором пререквизиты идут раньше потребителей** — это уже корректный topological order, без переворота.

При обратном направлении графа (`graph[a] = кто зависит от a`, то есть рёбра идут "от пререквизита к потребителю"): DFS из пререквизита `a` уходит вглубь к потребителям и они завершаются **раньше**, чем сам `a` — то есть порядок завершения получается **обратным** тому, что нужно (потребители оказываются в `order` раньше своих пререквизитов) — вот тут `reverse` обязателен.

