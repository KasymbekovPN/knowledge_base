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

---
---
---

- [x] Array/String - Two Sum (2026.07.18)
- [x] Array/String - Best Time to Buy and Sell Stock (2026.07.18)
- [x] Array/String - Maximum Subarray (Kadane's algorithm) (2026.07.18)
- [x] Array/String - Merge Intervals (2026.07.18)
- [x] Array/String - Product of Array Except Self (2026.07.19)
- [x] Array/String - Longest Substring Without Repeating Characters (2026.07.19)
- [x] Array/String - Group Anagrams (2026.07.19)
- [x] Array/String - Valid Parentheses (2026.07.19)
- [x] Array/String - Container With Most Water (2026.07.19)
- [x] Array/String - 3Sum (2026.07.19)
- [x] LinkedList - Reverse Linked List (2026.07.19)
- [x] LinkedList - Detect Cycle in Linked List (Floyd's) (2026.07.19)
- [x] LinkedList - Merge Two Sorted Lists (2026.07.21)
- [x] LinkedList - Remove Nth Node From End of List (2026.07.21)
- [x] LinkedList - LRU Cache (список + hash map) (2026.07.21)
- [x] Trees/Graphs - Maximum Depth of Binary Tree (2026.07.21)
- [x] Trees/Graphs - Validate Binary Search Tree (2026.07.21)
- [x] Trees/Graphs - Binary Tree Level Order Traversal (2026.07.21)
- [x] Trees/Graphs - Lowest Common Ancestor of a BST/BT (2026.07.21)
- [ ] Trees/Graphs - Serialize and Deserialize Binary Tree
- [ ] Trees/Graphs - Number of Islands (BFS/DFS)
- [ ] Trees/Graphs - Clone Graph
- [ ] Trees/Graphs - Course Schedule (topological sort)
- [ ] Trees/Graphs - Word Ladder (BFS)
- [ ] Dynamic - Climbing Stairs
- [ ] Dynamic - Coin Change
- [ ] Dynamic - Longest Common Subsequence
- [ ] Dynamic - Longest Increasing Subsequence
- [ ] Dynamic - Edit Distance
- [ ] Dynamic - House Robber
- [ ] Dynamic - Word Break
- [ ] Dynamic - 0/1 Knapsack
- [ ] hear/stack/queue - Min Stack
- [ ] hear/stack/queue - Kth Largest Element in an Array
- [ ] hear/stack/queue - Top K Frequent Elements
- [ ] hear/stack/queue - Sliding Window Maximum
- [ ] backtracking - Permutations
- [ ] backtracking - Subsets
- [ ] backtracking - N-Queens
- [ ] backtracking - Combination Sum


**Прочее (жадные, бинарный поиск, дизайн)** 41. Binary Search 42. Search in Rotated Sorted Array 43. Trapping Rain Water 44. Merge K Sorted Lists 45. Design a Trie (Prefix Tree)
