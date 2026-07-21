[[raw data/cpp/interview/_|<=]]

## Binary Tree Level Order Traversal

**Условие:** дан корень бинарного дерева. Вернуть обход по уровням — список списков, где каждый внутренний список содержит значения узлов одного уровня, слева направо, уровни идут сверху вниз (от корня к листьям).

### Идея

Это классическая задача на **BFS**. Используем очередь (`std::queue`). Ключевой приём — на каждой итерации внешнего цикла запоминаем `levelSize = q.size()` **до** того, как в очередь начнут добавляться узлы следующего уровня. Обрабатываем ровно `levelSize` узлов текущего уровня, собирая их значения в отдельный вектор, и параллельно добавляем в очередь их детей (которые образуют следующий уровень).

### Решение


```cpp
#include "binary_tree_level_order_traversal.h"  
  
#include <iostream>  
#include <format>  
#include <queue>  
  
namespace binary_tree_level_order_traversal {  
  
struct TreeNode {  
    int value;  
    TreeNode* left;  
    TreeNode* right;  
  
    explicit TreeNode(const int value):  
        value(value),  
        left(nullptr),  
        right(nullptr) {}  
};  
  
std::vector<std::vector<int>> level_order(const TreeNode* const node) {  
    std::vector<std::vector<int>> result;  
    if (!node) return result;  
  
    std::queue<const TreeNode*> q;  
    q.push(node);  
  
    while (!q.empty()) {  
        const int level_size{static_cast<int>(q.size())};  
        std::vector<int> level;  
        level.reserve(level_size);  
  
        for (int i{}; i < level_size; ++i) {  
            const auto current = q.front();  
            q.pop();  
            level.push_back(current->value);  
  
            if (current->left) q.push(current->left);  
            if (current->right) q.push(current->right);  
        }        result.push_back(std::move(level));  
    }  
    return result;  
}  
  
void dfs_helper(const TreeNode* const node, const int depth, std::vector<std::vector<int>>& result) {  
    if (!node) return;  
  
    if (depth == static_cast<int>(result.size())) {  
        result.push_back({});  
    }    result[depth].push_back(node->value);  
  
    dfs_helper(node->left, depth + 1, result);  
    dfs_helper(node->right, depth + 1, result);  
}  
  
std::vector<std::vector<int>> level_order_dfs(const TreeNode* const node) {  
    std::vector<std::vector<int>> result;  
    dfs_helper(node, 0, result);  
  
    return result;  
}  
  
static void delete_tree(const TreeNode* const node) {  
    if (node == nullptr) return;  
  
    delete_tree(node->left);  
    delete_tree(node->right);  
  
    delete node;  
}  
  
void demo() {  
    const auto root = new TreeNode(15);  
    root->left = new TreeNode(10);  
    root->right = new TreeNode(20);  
    root->left->left = new TreeNode(9);  
    root->right->left = new TreeNode(18);  
    root->right->right = new TreeNode(25);  
  
    for (const auto result = level_order(root); auto vec : result) {  
        std::cout << "[";  
        std::string delimiter;  
        for (const auto item: vec) {  
            std::cout << std::format("{}{}", delimiter, item);  
            delimiter = ", ";  
        }
		std::cout << "]\n";  
    }  
    for (const auto result = level_order_dfs(root); auto vec : result) {  
        std::cout << "[";  
        std::string delimiter;  
        for (const auto item: vec) {  
            std::cout << std::format("{}{}", delimiter, item);  
            delimiter = ", ";  
        }
		std::cout << "]\n";  
    }  
    delete_tree(root);  
}  
  
}
```

### Разбор

- `levelSize = q.size()` фиксирует **количество узлов именно текущего уровня** в момент начала итерации. Это критично: если не зафиксировать это число заранее, а просто проверять `!q.empty()` внутри внутреннего цикла, узлы следующего уровня (только что добавленные детьми) перемешаются с текущим — уровни "поплывут".
- Внутренний `for` обрабатывает ровно `levelSize` узлов — все они гарантированно принадлежат одному уровню, так как были в очереди до начала добавления их детей.
- `level.push_back(node->val)` — собираем значения текущего уровня; дети (`node->left`, `node->right`) добавляются в конец очереди и будут обработаны на следующей итерации внешнего цикла (то есть уже как следующий уровень).
- `result.push_back(std::move(level))` — перемещаем вектор уровня в результат, избегая копирования.

### Пример

```
        3
       / \
      9  20
         /  \
        15   7

q = [3]
Итерация 1: levelSize=1
  node=3: level=[3]; push(9), push(20)
  result = [[3]]
q = [9, 20]

Итерация 2: levelSize=2
  node=9:  level=[9];    (детей нет)
  node=20: level=[9,20]; push(15), push(7)
  result = [[3], [9,20]]
q = [15, 7]

Итерация 3: levelSize=2
  node=15: level=[15];
  node=7:  level=[15,7];
  result = [[3], [9,20], [15,7]]
q = []

Очередь пуста -> стоп.
Результат: [[3], [9,20], [15,7]]
```

### Сложность

- Время: **O(n)** — каждый узел попадает в очередь и обрабатывается ровно один раз.
- Память: **O(n)** — в худшем случае (последний уровень широкого дерева) очередь и результат хранят до половины всех узлов.

### Альтернатива через DFS с передачей глубины

Разбор: `depth == result.size()` — признак, что уровень `depth` встречается впервые, значит нужно создать под него новый вектор. Порядок обхода (сначала левое поддерево, потом правое) гарантирует, что внутри каждого уровня значения окажутся в правильном порядке слева направо.

- Время: O(n), память: O(h) под стек рекурсии (не считая результата) — экономнее по вспомогательной памяти на узких/глубоких деревьях, но менее интуитивно, чем BFS-версия.

### Частые вариации

- **Binary Tree Zigzag Level Order Traversal** — то же самое, но чётные уровни в обратном порядке (реверсировать вектор уровня или использовать deque и добавлять то в начало, то в конец).
- **Binary Tree Right Side View** — на каждом уровне брать только последний узел (правый край).
- **Average of Levels in Binary Tree** — вместо сбора значений считать среднее по каждому уровню.
