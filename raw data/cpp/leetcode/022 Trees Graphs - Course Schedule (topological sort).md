[[raw data/cpp/interview/_|<=]]

## Course Schedule (Topological Sort)

**Условие:** дано число курсов `numCourses` и список пар `prerequisites`, где `[a, b]` означает "чтобы взять курс `a`, нужно сначала пройти курс `b`". Определить, возможно ли пройти все курсы (то есть нет ли циклической зависимости).

### Идея

Это граф с направленными рёбрами (`b → a`, "b нужен перед a"). Задача сводится к вопросу: **есть ли цикл в направленном графе?** Если цикл есть — расписание невозможно (курсы циклически зависят друг от друга). Если цикла нет — граф является DAG (Directed Acyclic Graph), и его можно топологически отсортировать.

Два стандартных подхода: **Kahn's algorithm** (BFS по in-degree) и **DFS с трёхцветной маркировкой** для обнаружения циклов.

```cpp
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
```

### Решение 1: Kahn's algorithm (BFS)

Идея: курс без непройденных предпосылок (in-degree = 0) можно взять прямо сейчас. Берём все такие курсы, "убираем" их из графа (уменьшаем in-degree их зависимых курсов), это может открыть новые курсы с in-degree = 0 — добавляем их в очередь. Если в итоге удалось обработать все `numCourses` курсов — цикла нет.

### Разбор

- `adj[prereq]` — список курсов, которые становятся доступнее после прохождения `prereq` (ребро `prereq → course`).
- `inDegree[course]` — количество непройденных предпосылок для курса. Курс можно взять, когда это число доходит до 0.
- Начинаем с курсов, у которых `inDegree == 0` — их можно взять сразу без предпосылок.
- При "взятии" курса `cur` уменьшаем `inDegree` всех зависящих от него курсов; если у кого-то из них `inDegree` дошло до 0 — все его предпосылки теперь пройдены, добавляем его в очередь.
- Если в конце `processed == numCourses` — все курсы удалось "взять" по порядку, значит цикла нет. Если `processed < numCourses` — часть курсов осталась заблокированной друг на друге по кругу (цикл), их `inDegree` никогда не достигнет 0.

### Пример

```
numCourses=4, prerequisites=[[1,0],[2,0],[3,1],[3,2]]
(1 требует 0; 2 требует 0; 3 требует 1 и 2)

adj[0]=[1,2], adj[1]=[3], adj[2]=[3]
inDegree: [0]=0, [1]=1, [2]=1, [3]=2

q=[0] (только у 0 inDegree=0)

processed=1, cur=0: 
  next=1: inDegree[1]=0 -> push(1)
  next=2: inDegree[2]=0 -> push(2)
q=[1,2]

processed=2, cur=1:
  next=3: inDegree[3]=1 (ещё не 0)
q=[2]

processed=3, cur=2:
  next=3: inDegree[3]=0 -> push(3)
q=[3]

processed=4, cur=3: (нет исходящих)
q=[]

processed==4==numCourses -> true, можно пройти все курсы
```

Пример с циклом: `prerequisites=[[0,1],[1,0]]` — `inDegree[0]=1, inDegree[1]=1`, ни у одного `inDegree` не 0 изначально → очередь пуста с самого начала → `processed=0 != numCourses` → `false`.

### Сложность

- Время: **O(V + E)** — где `V = numCourses`, `E` — число рёбер (пар prerequisites).
- Память: **O(V + E)** — списки смежности, массив in-degree, очередь.

### Решение 2: DFS с трёхцветной маркировкой (альтернатива)

Идея: красим каждый узел в один из 3 цветов — `white` (не посещён), `gray` (в процессе обработки, находится в текущем стеке рекурсии), `black` (полностью обработан). Если при DFS встречаем узел, помеченный `gray` — это **обратное ребро**, то есть цикл.

**Разбор:** `Gray` — узел находится в текущей цепочке рекурсивных вызовов (ещё не "закрыт"). Если во время обхода соседей встречаем узел, уже помеченный `Gray`, — значит есть путь, ведущий обратно к предку в текущей ветке рекурсии, то есть цикл. `Black` — узел и всё его поддерево дерева обхода уже проверены и циклов не содержат, повторно заходить не нужно (в отличие от простого `visited`/`unvisited`, здесь именно различие `Gray` vs `Black` и ловит цикл, а не просто "уже видели").

- Время: **O(V + E)**, память: **O(V + E)**.

### Частые вариации

- **Course Schedule II** — вернуть саму топологическую сортировку (порядок прохождения курсов), а не просто true/false — тривиальное расширение Kahn's algorithm: добавлять `cur` в результирующий список при обработке.
- **Alien Dictionary** — построить порядок символов алфавита по отсортированному списку слов — топологическая сортировка на графе символов.
- **Minimum Height Trees** — похожий подход "снятия листьев по слоям" (аналог Kahn's algorithm), но для неориентированного дерева.
