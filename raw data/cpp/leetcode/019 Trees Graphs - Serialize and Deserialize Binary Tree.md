[[raw data/cpp/interview/_|<=]]

## Serialize and Deserialize Binary Tree

**Условие:** разработать алгоритм сериализации бинарного дерева в строку и десериализации строки обратно в дерево. Форма сериализации произвольна — лишь бы `deserialize(serialize(root))` восстанавливало дерево, идентичное исходному по структуре.

### Идея

Ключевая проблема — как закодировать **структуру** дерева (где именно `nullptr`), а не только значения. Простое перечисление значений в pre-order обходе неоднозначно без явных маркеров пустых поддеревьев: например, `[1,2,3]` можно интерпретировать как разные деревья, если не знать, где были `null`-узлы.

Решение — **pre-order DFS с явными маркерами `null`** для пустых поддеревьев. Записываем значение узла, затем рекурсивно сериализуем левое поддерево, затем правое; для `nullptr` пишем специальный маркер (например, `"#"`). Такая сериализация с явными `null`-маркерами однозначно восстанавливается, потому что pre-order обход с маркерами пустых узлов содержит полную информацию о структуре — при десериализации мы точно знаем, когда поддерево заканчивается.

### Решение

```cpp
#include "ser_des_binary_tree.h"  
  
#include <queue>  
#include <sstream>  
#include <iostream>  
#include <format>  
  
namespace ser_des_binary_tree {  
  
struct TreeNode {  
    int value;  
    TreeNode* left;  
    TreeNode* right;  
    explicit TreeNode(const int value):  
        value(value),  
        left(nullptr),  
        right(nullptr) {}  
};  
  
std::string Codec::serialize(TreeNode* root) {  
    std::ostringstream out;  
    serialize_helper(root, out);  
  
    return out.str();  
}  
  
TreeNode* Codec::deserialize(const std::string& data) {  
    std::istringstream in{data};  
    return deserialize_helper(in);  
}  
  
std::string Codec::serialize_bfs(TreeNode* root) {  
    std::ostringstream out;  
    std::queue<const TreeNode*> q;  
    q.push(root);  
  
    while (!q.empty()) {  
        const auto node = q.front();  
        q.pop();  
  
        if (node) {  
            out << node->value << " ";  
            q.push(node->left);  
            q.push(node->right);  
        } else {  
            out << "# ";  
        }
	}  
    return out.str();  
}  
  
TreeNode* Codec::deserialize_bfs(const std::string& data) {  
    std::istringstream in{data};  
    std::string token;  
    in >> token;  
    if (token == "#") return nullptr;  
  
    const auto root = new TreeNode{std::stoi(token)};  
    std::queue<TreeNode*> q;  
    q.push(root);  
  
    while (!q.empty()) {  
        TreeNode* node = q.front();  
        q.pop();  
  
        if (in >> token && token != "#") {  
            node->left = new TreeNode{std::stoi(token)};  
            q.push(node->left);  
        }        if (in >> token && token != "#") {  
            node->right = new TreeNode{std::stoi(token)};  
            q.push(node->right);  
        }
	}  
    return root;  
}  
  
void Codec::serialize_helper(TreeNode* root, std::ostringstream& out) {  
    if (!root) {  
        out << "# ";  
        return;  
    }  
    out << root->value << " ";  
    serialize_helper(root->left, out);  
    serialize_helper(root->right, out);  
}  
  
TreeNode* Codec::deserialize_helper(std::istringstream& in) {  
    std::string token;  
    in >> token;  
  
    if (token == "#") return nullptr;  
  
    TreeNode* node = new TreeNode{std::stoi(token)};  
    node->left = deserialize_helper(in);  
    node->right = deserialize_helper(in);  
  
    return node;  
}  
  
static void delete_tree(const TreeNode* const node) {  
    if (node == nullptr) return;  
  
    delete_tree(node->left);  
    delete_tree(node->right);  
  
    delete node;  
}  
  
void demo() {  
    //   1  
    //  / \    
    // 2   3    
    //    / \    
    //   4   5    
    const auto root = new TreeNode{1};  
    root->left = new TreeNode{2};  
    root->right = new TreeNode{3};  
    root->right->left = new TreeNode{4};  
    root->right->right = new TreeNode{5};  
  
    auto codec = Codec();  
    std::cout << std::format("serialize: {}\n", codec.serialize(root));  
    std::cout << std::format("serialize bfs: {}\n", codec.serialize_bfs(root));  
  
    delete_tree(root);  
}  
  
}
```

### Разбор

- `serializeHelper` — pre-order обход: сначала записываем значение текущего узла (или `"#"` для `nullptr`), затем рекурсивно сериализуем левое поддерево, затем правое. Пробел — разделитель токенов.
- `deserializeHelper` читает токены **в том же порядке**, в котором они были записаны (pre-order), поэтому рекурсивно восстанавливает дерево: читаем токен — если `"#"`, это пустое поддерево, возвращаем `nullptr`; иначе создаём узел с этим значением и рекурсивно строим его левое и правое поддеревья из **оставшихся** токенов потока.
- `std::istringstream` используется вместо ручного разбора индексов — оператор `>>` сам пропускает пробелы и последовательно извлекает токены, что упрощает код по сравнению с ручным сплитом строки.
- Важно, что `deserializeHelper` принимает поток **по ссылке** — так каждый рекурсивный вызов продолжает чтение с того места, где остановился предыдущий, а не начинает заново.

### Пример

```
Дерево:
        1
       / \
      2   3
         / \
        4   5

serialize:
  node=1: "1 "
    left(2): "1 2 "
      left(nullptr): "1 2 # "
      right(nullptr): "1 2 # # "
    right(3): "1 2 # # 3 "
      left(4): "1 2 # # 3 4 "
        left(nullptr): "1 2 # # 3 4 # "
        right(nullptr): "1 2 # # 3 4 # # "
      right(5): "1 2 # # 3 4 # # 5 "
        left/right(nullptr): "1 2 # # 3 4 # # 5 # # "

Результат: "1 2 # # 3 4 # # 5 # # "

deserialize("1 2 # # 3 4 # # 5 # # "):
  token="1" -> node(1)
    left: token="2" -> node(2)
      left: token="#" -> nullptr
      right: token="#" -> nullptr
    right: token="3" -> node(3)
      left: token="4" -> node(4)
        left: token="#" -> nullptr
        right: token="#" -> nullptr
      right: token="5" -> node(5)
        left: token="#" -> nullptr
        right: token="#" -> nullptr

Восстановлено то же дерево.
```

### Сложность

- Время: **O(n)** и для сериализации, и для десериализации — каждый узел (и каждый `null`-маркер) обрабатывается один раз.
- Память: **O(n)** — под строку сериализации и стек рекурсии (глубина O(h)).

### Альтернатива: BFS-сериализация (как в LeetCode-визуализации)

Тот же принцип, но по уровням (level-order), с `null`-маркерами — это формат, который часто визуализируют в интерфейсе LeetCode:

И pre-order-DFS, и BFS-варианты одинаково корректны и имеют одинаковую сложность — разница лишь в порядке обхода и, соответственно, порядке токенов в строке.

### Частые вариации

- **Serialize and Deserialize BST** — если известно, что дерево — BST, можно сериализовать **без** `null`-маркеров (только pre-order значения), т.к. упорядоченность BST однозначно восстанавливает структуру — экономия места, но требует отдельной логики восстановления через границы диапазона (как в Validate BST).
- **Serialize and Deserialize N-ary Tree** — аналогично, но нужно кодировать ещё и количество детей у каждого узла (или маркер конца списка детей).
- **Encode and Decode Strings** — родственная идея (кодирование с длиной/разделителем), но для списка строк, а не дерева.
