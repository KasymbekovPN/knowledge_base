[[raw data/cpp/interview/_|<=]]

## Lowest Common Ancestor (LCA) of a BST / Binary Tree

**Условие:** даны корень дерева и два узла `p`, `q` (гарантированно существующие в дереве, оба значения уникальны). Найти их наименьшего общего предка — самый глубокий узел, который является предком (или самим собой) для обоих `p` и `q`.

Разберём отдельно два варианта — для BST (проще, за счёт упорядоченности) и для произвольного бинарного дерева (общий случай).

### Часть 1: LCA в Binary Search Tree

**Идея:** используем свойство упорядоченности BST. Стартуем с корня. Если оба `p` и `q` меньше текущего узла — искомый LCA точно находится в левом поддереве (спускаемся влево). Если оба больше — спускаемся вправо. Как только значения `p` и `q` оказываются по разные стороны от текущего узла (или один из них равен текущему узлу) — текущий узел и есть LCA, точка "расхождения" путей к `p` и `q`.

```cpp
#include "lowest_common_ancestor.h"  
  
#include <iostream>  
#include <format>  
  
namespace lowest_common_ancestor {  
  
struct TreeNode {  
    int value;  
    TreeNode* left;  
    TreeNode* right;  
    explicit TreeNode(const int value):  
        value(value),  
        left(nullptr),  
        right(nullptr) {}  
};  
  
TreeNode* lowest_common_ancestor_bst(TreeNode* root, TreeNode* p, TreeNode* q) {  
    TreeNode* node{root};  
  
    while (node) {  
        if (p->value < node->value && q->value < node->value) {  
            node = node->left;  
        } else if (p->value > node->value && q->value > node->value) {  
            node = node->right;  
        } else {  
            return node;  
        }
	}  
    return nullptr;  
}  
  
TreeNode* lowest_common_ancestor(TreeNode* root, TreeNode* p, TreeNode* q) {  
    if (root == nullptr || root == p || root == q) return root;  
  
    TreeNode* left = lowest_common_ancestor(root->left, p, q);  
    TreeNode* right = lowest_common_ancestor(root->right, p, q);  
    if (left && right) {  
        return root;  
    }  
    return left ? left : right;  
}  
  
static void print_node(const TreeNode* const node) {  
    if (node) {  
        std::cout << std::format("[value: {}]\n", node->value);  
    } else {  
        std::cout << "NULL\n";  
    }
}  
  
static void delete_tree(const TreeNode* const node) {  
    if (node == nullptr) return;  
  
    delete_tree(node->left);  
    delete_tree(node->right);  
  
    delete node;  
}  
  
void demo() {  
    //     6  
    //    / \    
    //   2   8    
    //  / \ / \    
    // 0  4 7  9    
    //   / \    
    //  3   5    
    const auto root = new TreeNode(6);  
    root->left = new TreeNode(2);  
    root->right = new TreeNode(8);  
    const auto node0 = new TreeNode(0);  
    root->left->left = node0;  
    root->left->right = new TreeNode(4);  
    root->right->left = new TreeNode(7);  
    root->right->right = new TreeNode(9);  
    root->left->right->left = new TreeNode(3);  
    const auto node5 = new TreeNode(5);  
    root->left->right->right = node5;  
  
    print_node(lowest_common_ancestor_bst(root, node0, node5));  
    print_node(lowest_common_ancestor(root, node0, node5));  
  
    delete_tree(root);  
}  
  
}
```

**Разбор:** пока `p` и `q` оба меньше или оба больше текущего узла — они точно находятся в одном и том же поддереве, спускаемся туда. Как только это перестаёт выполняться (значения по разные стороны, либо одно из них равно `node->val`) — дальше спускаться некуда, это и есть точка расхождения = LCA.

**Пример:**

```
        6
       / \
      2   8
     / \ / \
    0  4 7  9
      / \
     3   5

p=2, q=8:
  node=6: 2<6 и 8<6? нет (8>6) -> расхождение -> LCA = 6

p=2, q=4:
  node=6: оба < 6 -> идём влево, node=2
  node=2: p=2==node(2) -> не оба <, не оба > -> LCA = 2
```

- Время: **O(h)**, память: **O(1)** (итеративно) — где `h` — высота дерева.

### Часть 2: LCA в произвольном Binary Tree (без упорядоченности)

**Идея:** свойства BST больше нет, упорядоченность не поможет. Используем рекурсивный DFS: для каждого узла спрашиваем, найден ли `p` или `q` где-то в левом поддереве и где-то в правом. Если оба поддерева "нашли" что-то (`left != nullptr && right != nullptr`) — значит текущий узел и есть точка расхождения, то есть LCA. Если только одно поддерево что-то нашло — передаём этот результат наверх без изменений.

**Разбор:**

- База рекурсии: `root == nullptr` — дошли до пустого поддерева, возвращаем `nullptr` (ничего не нашли). `root == p || root == q` — нашли один из искомых узлов, возвращаем его наверх сразу же (не углубляясь дальше, даже если второй узел мог бы быть у него в поддереве — этого достаточно, чтобы корректно определить LCA на уровне выше).
- Если и `left`, и `right` не `nullptr` — это значит `p` найден в одном поддереве, `q` — в другом (или один из них — сам текущий узел, а другой в одном из поддеревьев). Текущий узел — точка, где пути к `p` и `q` расходятся, то есть LCA.
- Если только одно из поддеревьев вернуло не-`nullptr` — оба искомых узла (или пока что найденный один) находятся в этом одном поддереве, значит LCA определится глубже, а текущий узел просто прокидывает результат наверх без изменений.

**Пример:**

```
        3
       / \
      5   1
     / \ / \
    6  2 0  8
      / \
     7   4

p=5, q=1:
  root=3: left=LCA(5,p=5,q=1) детально -> ..., right=LCA(1,...)
  Проще: на узле 3 — 5 находится слева (сам корень левого поддерева == p),
         1 находится справа (сам корень правого поддерева == q)
  left=5 (найден p), right=1 (найден q) -> оба не null -> LCA = 3

p=5, q=4:
  root=3: идём в left(5) и right(1)
    right(1): оба потомка (0,8) не содержат 5 или 4 -> вернёт nullptr
    left(5): root==p(5) -> сразу возвращаем 5, не спускаясь глубже
  left=5, right=nullptr -> возвращаем left -> LCA = 5
```

(Даже если бы 4 действительно было в поддереве 5, ответ корректен, т.к. как только нашли `p`, дальше вниз можно не проверять — на уровне выше это разрешится правильно.)

- Время: **O(n)** — в худшем случае обход всего дерева.
- Память: **O(h)** — глубина стека рекурсии.

### Частые вариации

- **LCA with parent pointers** — если у узлов есть указатель на родителя, задача сводится к поиску точки пересечения двух путей до корня (аналогично "Intersection of Two Linked Lists").
- **LCA of Deepest Leaves** — найти LCA не для двух заданных узлов, а для всех самых глубоких листьев дерева.
- **LCA in N-ary Tree** — обобщение общего подхода (часть 2) на дерево с произвольным числом детей — та же идея, но цикл по всем детям вместо `left`/`right`.
