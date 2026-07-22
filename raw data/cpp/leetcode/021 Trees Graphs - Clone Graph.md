[[raw data/cpp/interview/_|<=]]

## Clone Graph

**Условие:** дан узел неориентированного связного графа (каждый узел содержит значение и список соседей). Создать глубокую копию (клон) всего графа и вернуть клон стартового узла.

### Идея

Основная сложность — граф может содержать циклы (соседи ссылаются друг на друга), поэтому наивная рекурсия "клонировать узел → рекурсивно клонировать всех соседей" уйдёт в бесконечную рекурсию без отслеживания уже посещённых/склонированных узлов.

Решение: используем хеш-таблицу `оригинальный узел → клонированный узел`. Как только встречаем узел впервые — создаём его клон и **сразу** записываем в таблицу, прежде чем рекурсивно идти к его соседям. Это гарантирует, что при возврате к этому узлу через цикл в графе мы найдём уже существующий клон вместо повторного клонирования (и бесконечной рекурсии).

### Решение

```cpp
#include "clone_graph.h"  
  
#include <unordered_map>  
#include <queue>  
#include <vector>  
#include <iostream>  
#include <format>  
#include <unordered_set>  
  
namespace clone_graph {  
  
struct Node {  
    int value;  
    std::vector<Node*> neighbors;  
    explicit Node(const int value):  
        value(value) {}  
};  
  
Node* clone_graph_dfs(Node* node) {  
    std::unordered_map<Node*, Node*> visited;  
    return clone_graph_dfs_impl(node, visited);  
}  
  
Node* clone_graph_dfs_impl(Node* node, std::unordered_map<Node*, Node*>& visited) {  
    if (!node) return nullptr;  
  
    if (const auto it = visited.find(node); it != visited.end()) {  
        // уже клонирован ранее -> возвращаем существующий клон  
        return it->second;  
    }  
    const auto clone = new Node{node->value};  
    visited[node] = clone;  
  
    for (Node* neighbor : node->neighbors) {  
        clone->neighbors.push_back(clone_graph_dfs_impl(neighbor, visited));  
    }  
    return clone;  
}  
  
Node* clone_graph_bfs(Node* node) {  
    if (!node) return nullptr;  
  
    std::unordered_map<Node*, Node*> visited;  
    visited[node] = new Node{node->value};  
  
    std::queue<Node*> q;  
    q.push(node);  
  
    while (!q.empty()) {  
        Node* current{q.front()};  
        q.pop();  
  
        for (Node* neighbor : current->neighbors) {  
            if (!visited.contains(neighbor)) {  
                visited[neighbor] = new Node{neighbor->value};  
                q.push(neighbor);  
            }           
            visited[current]->neighbors.push_back(visited[neighbor]);  
        }
	}  

    return visited[node];  
}  
  
static void print_node(const Node* const node) {  
    std::cout << node << std::format(" [value: {}] (", node->value);  
    for (const auto& neighbor : node->neighbors) {  
        std::cout << std::format("[value: {}]", neighbor->value);  
    }    
    std::cout << ")\n";  
}  
  
static void collector_nodes(Node* node, std::unordered_set<Node*>& collection) {  
    if (!node) return;  
  
    if (!collection.contains(node)) {  
        collection.insert(node);  
        for (const auto neighbor : node->neighbors) {  
            collector_nodes(neighbor, collection);  
        }        
        node->neighbors.clear();  
    }}  
  
static void delete_graphs(Node* node) {  
    std::unordered_set<Node*> collection;  
    collector_nodes(node, collection);  
  
    for (const auto item : collection) {  
        delete item;  
    }}  
  
void demo() {  
    // Граф: 1 -- 2  
    //       |    |
	//       4 -- 3    
	const auto n1 = new Node{1};  
    const auto n2 = new Node{2};  
    const auto n3 = new Node{3};  
    const auto n4 = new Node{4};  
  
    n1->neighbors.push_back(n2);  
    n1->neighbors.push_back(n4);  
  
    n2->neighbors.push_back(n1);  
    n2->neighbors.push_back(n3);  
  
    n3->neighbors.push_back(n2);  
    n3->neighbors.push_back(n4);  
  
    n4->neighbors.push_back(n1);  
    n4->neighbors.push_back(n3);  
  
    print_node(n1);  
    print_node(n2);  
    print_node(n3);  
    print_node(n4);  
  
    const auto cloned_bfs = clone_graph_bfs(n1);  
    print_node(cloned_bfs);  
  
    const auto cloned_dfs = clone_graph_dfs(n1);  
    print_node(cloned_dfs);  
  
    delete_graphs(n1);  
    delete_graphs(cloned_bfs);  
    delete_graphs(cloned_dfs);  
}  
  
}
```


### Решение (DFS)

### Разбор

- `visited` хранит соответствие `оригинал → клон`, играя двойную роль: (1) отслеживание "уже посещённых" узлов, (2) быстрый доступ к уже созданному клону для правильной сшивки соседей.
- Критичный момент: `visited[node] = clone` выполняется **сразу после создания клона, до цикла по соседям**. Если бы это было после — при циклической ссылке (сосед ссылается обратно на текущий узел) рекурсия бы не нашла запись в `visited` и создала бесконечную цепочку новых клонов.
- Для каждого соседа рекурсивно получаем его клон (либо создаём новый, либо берём уже существующий из `visited`) и добавляем в список соседей текущего клона.

### Пример

```
Граф: 1 -- 2
      |    |
      4 -- 3
(1 связан с 2 и 4; 2 связан с 1 и 3; 3 связан с 2 и 4; 4 связан с 1 и 3)

cloneGraph(1):
  visited={} -> clone(1) создан, visited={1:clone1}
  neighbors узла 1: [2, 4]
    cloneGraphDFS(2):
      visited={1:clone1} -> clone(2) создан, visited={1:clone1, 2:clone2}
      neighbors узла 2: [1, 3]
        cloneGraphDFS(1): найден в visited -> вернуть clone1 (не создаём заново!)
        cloneGraphDFS(3):
          clone(3) создан, visited={...,3:clone3}
          neighbors узла 3: [2, 4]
            cloneGraphDFS(2): найден в visited -> вернуть clone2
            cloneGraphDFS(4):
              clone(4) создан, visited={...,4:clone4}
              neighbors узла 4: [1, 3]
                cloneGraphDFS(1): найден -> clone1
                cloneGraphDFS(3): найден -> clone3
              clone4.neighbors = [clone1, clone3]
          clone3.neighbors = [clone2, clone4]
      clone2.neighbors = [clone1, clone3]
    clone1.neighbors = [clone2, clone4]

Результат: клон графа с той же структурой, все узлы — новые объекты.
```

### Сложность

- Время: **O(V + E)** — каждый узел клонируется один раз (V), каждое ребро обрабатывается при сшивке соседей (E).
- Память: **O(V)** — под хеш-таблицу и стек рекурсии.

### Решение (BFS, итеративное)

Если нужно избежать глубокой рекурсии (граф может быть очень большим):

Разбор: та же идея с хеш-таблицей `visited`, но обход соседей — через очередь, а не рекурсию. Клон соседа создаётся (если ещё не существует) сразу при первой встрече, до того как он попадёт в очередь на обработку — это и предотвращает повторное создание клонов при возврате через цикл графа.

- Время: **O(V + E)**, память: **O(V)** — аналогично DFS-версии, но без риска переполнения стека рекурсии на очень глубоких/больших графах.

### Частые вариации

- **Copy List with Random Pointer** — та же идея хеш-таблицы `оригинал → клон`, но применительно к связному списку с дополнительным произвольным указателем `random`.
- **Course Schedule** — тоже граф, но с направленными рёбрами, задача не на клонирование, а на обнаружение циклов (топологическая сортировка).
- **Graph Valid Tree** — проверка, что граф является деревом (связный и без циклов) — использует похожий обход, но с подсчётом рёбер/компонент.
