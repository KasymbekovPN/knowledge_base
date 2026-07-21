[[raw data/cpp/interview/_|<=]]

## Maximum Depth of Binary Tree

**Условие:** дан корень бинарного дерева. Найти его максимальную глубину — количество узлов на самом длинном пути от корня до листа.

### Идея

Классическая задача на рекурсию (DFS): глубина дерева с корнем `root` = `1 + max(глубина левого поддерева, глубина правого поддерева)`. База рекурсии — пустое дерево (`nullptr`) имеет глубину 0.

```cpp
#include "max_depth_binary_tree.h"  
  
#include <iostream>  
#include <format>  
#include <algorithm>  
#include <queue>  
#include <stack>  
#include <utility>  
  
namespace max_depth_binary_tree {  
  
struct TreeNode {  
    int value;  
    TreeNode* left;  
    TreeNode* right;  
    explicit TreeNode(const int value):  
        value(value),  
        left(nullptr),  
        right(nullptr) {}  
};  
  
int max_depth_recursive(const TreeNode* node) {  
    if (node == nullptr) return 0;  
  
    return 1 + std::max(  
        max_depth_recursive(node->left),  
        max_depth_recursive(node->right));  
}  
  
int max_depth_bfs(const TreeNode* node) {  
    if (node == nullptr) return 0;  
  
    std::queue<const TreeNode*> q;  
    q.push(node);  
    int depth{};  
  
    while (!q.empty()) {  
        const int level_size{static_cast<int>(q.size())};  
        for (int i{}; i < level_size; ++i) {  
            const TreeNode* current{q.front()};  
            q.pop();  
  
            if (current->left) q.push(current->left);  
            if (current->right) q.push(current->right);  
        }        ++depth;  
    }  
    return depth;  
}  
  
int max_depth_dfs(const TreeNode* node) {  
    if (node == nullptr) return 0;  
  
    std::stack<std::pair<const TreeNode*, int>> stk;  
    stk.push({node, 1});  
    int best{};  
  
    while (!stk.empty()) {  
        auto [current, depth] = stk.top();  
        stk.pop();  
  
        best = std::max(best, depth);  
  
        if (current->left) stk.push({current->left, depth + 1});  
        if (current->right) stk.push({current->right, depth + 1});  
    }  
    return best;  
}  
  
static void delete_tree(const TreeNode* node) {  
    if (node == nullptr) return;  
  
    delete_tree(node->left);  
    delete_tree(node->right);  
  
    delete node;  
}  
  
void demo() {  
    const auto root = new TreeNode(3);  
    root->left = new TreeNode(9);  
    root->right = new TreeNode(20);  
    root->right->left = new TreeNode(15);  
    root->right->right = new TreeNode(7);  
  
    std::cout << std::format("REC {}\n", max_depth_recursive(root));  
    std::cout << std::format("BFS {}\n", max_depth_bfs(root));  
    std::cout << std::format("DFS {}\n", max_depth_dfs(root));  
  
    delete_tree(root);  
}  
  
}
```


---
### Рекурсивное решение (DFS)

### Разбор

- База: пустое поддерево (`nullptr`) — глубина 0, дальше спускаться некуда.
- Рекурсивный переход: считаем глубину обоих поддеревьев независимо, берём максимум и добавляем 1 за сам текущий узел.
- Каждый узел посещается ровно один раз.

### Пример

```
        3
       / \
      9  20
         /  \
        15   7

maxDepth(3) = 1 + max(maxDepth(9), maxDepth(20))
maxDepth(9) = 1 + max(maxDepth(nullptr), maxDepth(nullptr)) = 1 + max(0,0) = 1
maxDepth(20) = 1 + max(maxDepth(15), maxDepth(7))
maxDepth(15) = 1 + max(0,0) = 1
maxDepth(7)  = 1 + max(0,0) = 1
maxDepth(20) = 1 + max(1,1) = 2
maxDepth(3)  = 1 + max(1,2) = 3

Ответ: 3
```

### Сложность

- Время: **O(n)** — каждый узел посещается один раз.
- Память: **O(h)** — глубина стека рекурсии равна высоте дерева `h` (в худшем случае вырожденное дерево — O(n), в среднем сбалансированное — O(log n)).

### Итеративное решение (BFS по уровням)

Иногда просят реализовать без рекурсии, чтобы избежать переполнения стека на очень глубоких деревьях.

Разбор: `levelSize` фиксирует, сколько узлов принадлежит текущему уровню **до** того, как в очередь добавятся узлы следующего уровня — это ключевой приём для поуровневого обхода (level order traversal). После обработки всех узлов уровня `depth` увеличивается на 1.

- Время: **O(n)**.
- Память: **O(w)**, где `w` — максимальная ширина дерева (может быть до O(n) для широкого дерева, например, полностью заполненного последнего уровня).

### Итеративное решение (DFS со стеком, эмуляция рекурсии)

### Частые вариации

- **Minimum Depth of Binary Tree** — аналогично, но минимум, и важен нюанс: если у узла только один потомок, минимальная глубина считается через существующего потомка, а не 0 (иначе некорректно для несимметричных деревьев).
- **Balanced Binary Tree** — проверка, что для каждого узла разница высот левого/правого поддерева не превышает 1; решается похожей рекурсией, но возвращающей -1 как признак "уже разбалансировано", чтобы не считать высоту дальше зря.
- **Diameter of Binary Tree** — самый длинный путь между двумя листьями (не обязательно через корень) — считается через ту же рекурсию высоты, но с побочным обновлением глобального максимума `лево + право` на каждом узле.
