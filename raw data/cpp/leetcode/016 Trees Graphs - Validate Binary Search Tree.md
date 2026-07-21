[[raw data/cpp/interview/_|<=]]

## Validate Binary Search Tree

**Условие:** дан корень бинарного дерева. Проверить, является ли оно корректным деревом поиска (BST): для каждого узла все значения в левом поддереве строго меньше значения узла, все значения в правом поддереве строго больше, и это свойство рекурсивно выполняется для всех поддеревьев (а не только для непосредственных потомков).

### Идея

**Частая ошибка** — проверять только `left->val < node->val < right->val` для непосредственных детей. Это неверно: узел в глубине левого поддерева может быть больше корня, нарушая BST-свойство глобально, даже если локально с родителем всё в порядке.

Пример, где наивная проверка соседних узлов ошибочно скажет "valid", хотя дерево невалидно:

```
	   10
      /  \
     5    15
         /  \
        6    20
```

Здесь `10 < 15` (локально ок), но `10` находится в правом поддереве корня `15`, а `6` (в поддереве `10`) должно быть `> 15` — не является. Проверка только соседних пар это не поймает.

**Правильный подход:** передавать вниз по рекурсии допустимый диапазон `(lower, upper)`, в который должно попадать значение каждого узла. При спуске влево — сужаем верхнюю границу до значения родителя; при спуске вправо — сужаем нижнюю границу.

### Решение

```cpp
#include "validate_bst.h"  
  
#include <limits>  
#include <iostream>  
#include <format>  
#include <optional>  
  
namespace validate_bst {  
  
struct TreeNode {  
    int value;  
    TreeNode* left;  
    TreeNode* right;  
    explicit TreeNode(const int value):  
        value(value),  
        left(nullptr),  
        right(nullptr) {}  
};  
  
bool is_valid(const TreeNode* const node, const long long lower, const long long upper) {  
    if (!node) return true;  
  
    if (node->value <= lower || node->value >= upper) return false;  
  
    return is_valid(node->left, lower, node->value) &&  
        is_valid(node->right, node->value, upper);  
}  
  
bool is_valid_in_order(const TreeNode* const node, std::optional<long long>& prev) {  
    if (!node) return true;  
  
    if (!is_valid_in_order(node->left, prev)) return false;  
  
    if (prev.has_value() && node->value <= prev.value()) {  
        return false;  
    }    prev = node->value;;  
  
    return is_valid_in_order(node->right, prev);  
}  
  
static void delete_tree(const TreeNode* const node) {  
    if (node == nullptr) return;  
  
    delete_tree(node->left);  
    delete_tree(node->right);  
  
    delete node;  
}  
  
void demo() {  
    const auto good_root = new TreeNode(15);  
    good_root->left = new TreeNode(10);  
    good_root->right = new TreeNode(20);  
    good_root->left->left = new TreeNode(9);  
    good_root->right->left = new TreeNode(18);  
    good_root->right->right = new TreeNode(25);  
  
    const auto bad_root = new TreeNode(10);  
    bad_root->left = new TreeNode(5);  
    bad_root->right = new TreeNode(15);  
    bad_root->right->left = new TreeNode(6);  
    bad_root->right->right = new TreeNode(20);  
  
    std::cout << std::format("good is_valid {}\n",is_valid(  
        good_root,  
        std::numeric_limits<long long>::min(),  
        std::numeric_limits<long long>::max()));  
  
    std::cout << std::format("bad is_valid {}\n",is_valid(  
        bad_root,  
        std::numeric_limits<long long>::min(),  
        std::numeric_limits<long long>::max()));  
  
    std::optional<long long> good_prev;  
    std::cout << std::format("good is_valid_in_order {}\n", is_valid_in_order(  
        good_root,  
        good_prev));  
  
    std::optional<long long> bad_prev;  
    std::cout << std::format("bad is_valid_in_order {}\n", is_valid_in_order(  
        bad_root,  
        bad_prev));  
  
    delete_tree(good_root);  
    delete_tree(bad_root);  
}  
  
  
}
```

### Разбор

- `lower` и `upper` — открытый интервал `(lower, upper)`, в который обязано попадать значение текущего узла.
- Тип `long long` для границ, а не `int`, — важный нюанс: если значения в дереве могут доходить до `INT_MIN`/`INT_MAX`, использование `int` для границ сделает невозможным выразить "строго меньше `INT_MIN`" или "строго больше `INT_MAX`" — понадобится более широкий тип, иначе граничные случаи сломаются (типичная ловушка на собеседовании).
- При спуске влево: верхняя граница ужимается до `node->val` (всё в левом поддереве должно быть строго меньше текущего узла), нижняя граница не меняется.
- При спуске вправо: нижняя граница ужимается до `node->val`, верхняя не меняется.
- Условие `<=`/`>=` (не строгое `<`/`>` в проверке) отражает, что диапазон открытый: значение должно быть строго между границами, дубликаты запрещены (в классическом BST равные значения не допускаются).

### Пример

```
validateHelper(10, -inf, +inf): 10 ok left:
validateHelper(5, -inf, 10) -> 5 ok right:
validateHelper(15, 10, +inf) -> 15 ok left:
validateHelper(6, 10, 15) -> 6 <= 10 -> false!
```

### Сложность

- Время: **O(n)** — каждый узел посещается ровно один раз.
- Память: **O(h)** — глубина стека рекурсии.

### Альтернативное решение: in-order обход

Ключевое свойство BST: **in-order обход (лево → узел → право) корректного BST даёт строго возрастающую последовательность значений**. Достаточно сделать in-order обход и проверить, что каждое следующее значение строго больше предыдущего — не нужно хранить весь список значений, достаточно помнить последнее увиденное.

Разбор: `prev` хранит значение последнего посещённого узла в порядке in-order обхода. Если текущее значение не строго больше предыдущего — BST-свойство нарушено. `std::optional` элегантно решает проблему "ещё не было предыдущего значения" (первый посещённый узел), не прибегая к сентинелам вроде `INT_MIN - 1`.

- Время: **O(n)**, память: **O(h)** — аналогично первому подходу.

### Частые вариации

- **Recover Binary Search Tree** — два узла в BST случайно переставлены местами, нужно найти их и поменять обратно — решается через in-order обход с отслеживанием "нарушений" порядка.
- **Kth Smallest Element in a BST** — in-order обход даёт отсортированный порядок, останавливаемся на k-м элементе.
- **Convert Sorted Array to Binary Search Tree** — обратная задача, построение сбалансированного BST из отсортированного массива.
