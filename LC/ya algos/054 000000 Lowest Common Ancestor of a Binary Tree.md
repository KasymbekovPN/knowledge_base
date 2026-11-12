Задача 6 блока 7: Lowest Common Ancestor of a Binary Tree. Собираю тесты и проверяю.

Задача 6 блока 7 готова. Проверка по запуску: заглушка даёт 48 FAIL из 48, два разных эталона (рекурсия и отображение «узел → родитель») проходят все 48 без предупреждений, чисто под ASan/UBSan, три неверных варианта (всегда корень, поиск только слева, расчёт как для дерева поиска по значениям) ловятся.

## Lowest Common Ancestor of a Binary Tree

Дано бинарное дерево (не дерево поиска) с различными значениями и два разных узла `p` и `q`, оба точно есть в дереве. Верните их наименьшего общего предка: самый глубокий узел, у которого `p` и `q` лежат в поддереве. Сам узел считается своим предком, то есть если `p` лежит в поддереве `q`, ответом будет `q`.

```cpp
const TreeNode* lowest_common_ancestor(const TreeNode* root, const TreeNode* p, const TreeNode* q);
```

**Ограничения:** 2 ≤ n ≤ 10⁵, значения различны, `p != q`, оба принадлежат дереву. Узлы сравниваются как указатели.

**Вопросы интервьюера:**

1. Как работает ваш алгоритм и что возвращает рекурсивный вызов для поддерева, в котором нет ни `p`, ни `q`, ни одного из них, одного из них, обоих?
2. Почему случай «`p` предок `q`» не требует отдельной обработки?
3. Какая сложность по времени и по памяти? Как поведёт себя рекурсия на цепочке из 10⁵ узлов и как её обойти?
4. Как решить через родителей (отображение «узел → родитель»)? Сколько памяти это стоит?
5. Что изменится, если дерево было бы деревом поиска?

**Цель:** O(n) по времени, O(h) по памяти. **Ориентир по времени:** 15 минут.

```cpp
namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	const TreeNode* lowest_common_ancestor([[maybe_unused]] const TreeNode* root,
										   [[maybe_unused]] const TreeNode* p,
										   [[maybe_unused]] const TreeNode* q) {
		return nullptr;
	}

	// builds a tree from a level-order list, `nil` marks a missing child;
	// the nodes live in `pool`, which must not be reallocated afterwards
	TreeNode* build_tree(const std::vector<int>& level, std::vector<TreeNode>& pool) {
		if (level.empty() || level[0] == nil) {
			return nullptr;
		}
		pool.reserve(level.size());
		pool.push_back(TreeNode{level[0], nullptr, nullptr});
		std::queue<TreeNode*> parents;
		parents.push(&pool.back());
		std::size_t next{1};
		while (!parents.empty() && next < level.size()) {
			TreeNode* parent{parents.front()};
			parents.pop();
			if (next < level.size() && level[next] != nil) {
				pool.push_back(TreeNode{level[next], nullptr, nullptr});
				parent->left = &pool.back();
				parents.push(parent->left);
			}
			++next;
			if (next < level.size() && level[next] != nil) {
				pool.push_back(TreeNode{level[next], nullptr, nullptr});
				parent->right = &pool.back();
				parents.push(parent->right);
			}
			++next;
		}
		return &pool.front();
	}

	const TreeNode* find_by_value(const std::vector<TreeNode>& pool, int value) {
		for (const TreeNode& node : pool) {
			if (node.val == value) {
				return &node;
			}
		}
		return nullptr;
	}

	// the values in `level` are distinct, p and q are values of two different nodes
	void test_lca(const std::string& label,
				  const std::vector<int>& level,
				  int p,
				  int q,
				  int expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		const TreeNode* result{lowest_common_ancestor(root, find_by_value(pool, p), find_by_value(pool, q))};
		std::cout << std::format("{}: {}\n",
			label,
			result == find_by_value(pool, expected) ? "OK" : "FAIL") << std::flush;
	}

	// a tree whose nodes are numbered 0..n-1, the node 0 is the root, the value of a node is its number;
	// the parent of a node always has a smaller number than the node
	struct Tree {
		std::vector<TreeNode> pool;
		std::vector<std::size_t> parent;
		std::vector<std::size_t> depth;
	};

	// reference answer for large trees: climb from the deeper node to the same depth, then both together
	std::size_t expected_lca(const Tree& tree, std::size_t a, std::size_t b) {
		while (tree.depth[a] > tree.depth[b]) a = tree.parent[a];
		while (tree.depth[b] > tree.depth[a]) b = tree.parent[b];
		while (a != b) {
			a = tree.parent[a];
			b = tree.parent[b];
		}
		return a;
	}

	void test_big(const std::string& label, Tree& tree, std::size_t p, std::size_t q) {
		const TreeNode* expected{&tree.pool[expected_lca(tree, p, q)]};

		const auto start{std::chrono::steady_clock::now()};
		const TreeNode* result{lowest_common_ancestor(&tree.pool.front(), &tree.pool[p], &tree.pool[q])};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	Tree make_tree(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		Tree tree{std::vector<TreeNode>(count), std::vector<std::size_t>(count, 0), std::vector<std::size_t>(count, 0)};
		for (std::size_t i{0}; i < count; ++i) {
			tree.pool[i].val = static_cast<int>(i);
		}
		std::mt19937 gen{seed};
		std::vector<std::pair<std::size_t, bool>> free_places{{0, true}, {0, false}};
		for (std::size_t i{1}; i < count; ++i) {
			std::size_t parent{0};
			bool is_left{true};
			switch (shape) {
				case Shape::complete:
					parent = (i - 1) / 2;
					is_left = i % 2 == 1;
					break;
				case Shape::left_chain:
					parent = i - 1;
					break;
				case Shape::right_chain:
					parent = i - 1;
					is_left = false;
					break;
				case Shape::zigzag:
					parent = i - 1;
					is_left = i % 2 == 1;
					break;
				case Shape::random: {
					const std::size_t pick{gen() % free_places.size()};
					std::tie(parent, is_left) = free_places[pick];
					free_places[pick] = free_places.back();
					free_places.pop_back();
					free_places.push_back({i, true});
					free_places.push_back({i, false});
					break;
				}
			}
			(is_left ? tree.pool[parent].left : tree.pool[parent].right) = &tree.pool[i];
			tree.parent[i] = parent;
			tree.depth[i] = tree.depth[parent] + 1;
		}
		return tree;
	}

	// the node with the largest depth
	std::size_t deepest(const Tree& tree) {
		return static_cast<std::size_t>(std::max_element(tree.depth.begin(), tree.depth.end()) - tree.depth.begin());
	}
}

int main() {

	test_lca("description example 1, p and q are in different subtrees",
		{3, 5, 1, 6, 2, 0, 8, nil, nil, 7, 4},
		5, 1, 3);
	test_lca("description example 2, p is an ancestor of q",
		{3, 5, 1, 6, 2, 0, 8, nil, nil, 7, 4},
		5, 4, 5);
	test_lca("two nodes, the root is p",
		{1, 2},
		1, 2, 1);
	test_lca("two nodes, the root is q",
		{1, nil, 2},
		2, 1, 1);
	test_lca("root with two leaves",
		{1, 2, 3},
		2, 3, 1);
	test_lca("root with two leaves, reversed arguments",
		{1, 2, 3},
		3, 2, 1);
	test_lca("siblings deep inside the left subtree",
		{1, 2, 3, 4, 5, nil, nil, 6, 7},
		6, 7, 4);
	test_lca("siblings deep inside the right subtree",
		{1, 2, 3, nil, nil, 4, 5, nil, nil, 6, 7},
		6, 7, 5);
	test_lca("cousins under different parents",
		{1, 2, 3, 4, 5, 6, 7},
		4, 6, 1);
	test_lca("cousins under the same grandparent",
		{1, 2, 3, 4, 5, 6, 7},
		4, 5, 2);
	test_lca("both nodes in the left subtree, the ancestor is below the root",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		8, 11, 2);
	test_lca("both nodes in the right subtree, the ancestor is below the root",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		12, 15, 3);
	test_lca("the nodes are at different depths, p is deeper",
		{1, 2, 3, 4, 5, 6, 7, 8, 9},
		9, 3, 1);
	test_lca("the nodes are at different depths, q is deeper",
		{1, 2, 3, 4, 5, 6, 7, 8, 9},
		3, 9, 1);
	test_lca("p is the parent of q",
		{1, 2, 3, 4, 5, 6, 7},
		2, 5, 2);
	test_lca("q is the parent of p",
		{1, 2, 3, 4, 5, 6, 7},
		5, 2, 2);
	test_lca("p is the grandparent of q",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		3, 14, 3);
	test_lca("p is the root, q is deep",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		1, 15, 1);
	test_lca("q is the root, p is deep",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		15, 1, 1);
	test_lca("left chain, p and q are the deepest nodes",
		{1, 2, nil, 3, nil, 4, nil, 5},
		4, 5, 4);
	test_lca("left chain, p is the top, q is the bottom",
		{1, 2, nil, 3, nil, 4, nil, 5},
		2, 5, 2);
	test_lca("right chain, p is in the middle",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		3, 5, 3);
	test_lca("zigzag, the answer is in the middle",
		{10, 5, nil, nil, 8, 7},
		7, 8, 8);
	test_lca("zigzag, the nodes are the first and the last",
		{10, 5, nil, nil, 8, nil, 11},
		5, 11, 5);
	test_lca("the value zero is among the nodes",
		{0, -1, 1},
		-1, 1, 0);
	test_lca("the value zero is the answer",
		{5, 0, nil, -3, 7},
		-3, 7, 0);
	test_lca("negative values, the answer is the left child",
		{-1, -2, -3, -4, -5},
		-4, -5, -2);
	test_lca("a node with a single child is the answer",
		{1, 2, 3, nil, 4, 5, nil, nil, 6},
		4, 6, 4);
	test_lca("the second subtree is much deeper",
		{1, 2, 3, nil, nil, 4, nil, nil, 5, nil, 6, nil, 7},
		4, 7, 4);
	test_lca("a leaf and the sibling of its ancestor",
		{1, 2, 3, 4, 5, nil, nil, 6},
		6, 3, 1);
	test_lca("random 1, n = 3",
		{72, 26, -6},
		-6, 72, 72);
	test_lca("random 2, n = 5",
		{92, nil, 49, -23, nil, 44, nil, -60},
		49, -60, 49);
	test_lca("random 3, n = 8",
		{-46, 31, -64, nil, nil, 1, -76, nil, 77, nil, -88, nil, nil, -87},
		-88, 31, -46);
	test_lca("random 4, n = 12",
		{16, -68, -38, nil, 21, -98, -58, nil, nil, 72, 4, nil, -57, 42, 25, nil, nil, nil, nil, nil, nil, -16},
		21, -57, 16);
	test_lca("random 5, n = 20",
		{4, 45, 96, nil, -13, nil, 75, 83, -87, 55, -21, 67, nil, 46, -54, nil, 99, nil, 16, nil, nil, nil, 93, nil, -3, nil, 38, -69, 89, nil, nil, nil, nil, nil, nil, nil, nil, nil, -77},
		4, 46, 4);
	test_lca("random 6, n = 30",
		{-46, 27, -30, nil, nil, 100, 86, -44, -3, -99, -58, nil, -49, -61, 65, 2, nil, -85, nil, -43, -79, -100, nil, 32, nil, 50, nil, nil, nil, 35, nil, 12, 8, -35, 66, nil, nil, 62, nil, nil, nil, 74, 61, -45, nil, nil, nil, 48, nil, nil, nil, nil, nil, -93},
		-3, 65, -3);
	test_lca("random 7, n = 40",
		{60, -50, 70, -91, -97, -12, -74, -87, -67, -80, 16, -19, 82, -14, 38, -33, nil, nil, -9, nil, nil, nil, nil, 87, -81, nil, -62, 22, nil, nil, -99, -22, -55, nil, nil, nil, nil, -73, nil, nil, nil, nil, nil, nil, 96, 72, -24, nil, -25, nil, nil, nil, nil, nil, nil, -52, nil, 48, nil, nil, -78, -23, -69, 32, nil, nil, nil, 83, nil, -95, nil, -35, nil, -29, nil, nil, 63},
		-24, -78, -24);
	test_lca("random 8, n = 50",
		{-75, 57, 27, -16, -31, nil, 3, nil, 100, -33, 68, -82, -92, nil, 42, 28, 51, -81, -17, -44, -22, nil, -27, nil, nil, -38, -40, 33, 40, nil, nil, 94, nil, -19, nil, -48, 19, nil, nil, 58, -18, 73, -8, -73, 25, nil, nil, nil, nil, -45, nil, nil, nil, nil, -23, -72, 75, nil, nil, nil, -35, nil, -55, nil, nil, -7, -30, -4, nil, nil, nil, nil, nil, 85, nil, 91, nil, nil, -49, nil, nil, nil, nil, -74, 9, nil, nil, nil, 90, 4, nil, nil, nil, nil, nil, nil, nil, -46},
		-35, -23, -75);

	{
		Tree tree{make_tree(Shape::complete, 100000, 0)};
		test_big("n = 100000, complete tree, two last leaves", tree, 99999, 99998);
	}
	{
		Tree tree{make_tree(Shape::complete, 100000, 0)};
		test_big("n = 100000, complete tree, leaves of different subtrees of the root", tree, 99999, 50000);
	}
	{
		Tree tree{make_tree(Shape::complete, 100000, 0)};
		test_big("n = 100000, complete tree, a leaf and its distant ancestor", tree, 99999, 3);
	}
	{
		Tree tree{make_tree(Shape::left_chain, 100000, 0)};
		test_big("n = 100000, left chain, the deepest node and the middle one", tree, 99999, 50000);
	}
	{
		Tree tree{make_tree(Shape::left_chain, 100000, 0)};
		test_big("n = 100000, left chain, the deepest node and the root", tree, 99999, 0);
	}
	{
		Tree tree{make_tree(Shape::left_chain, 100000, 0)};
		test_big("n = 100000, left chain, two deepest nodes", tree, 99999, 99998);
	}
	{
		Tree tree{make_tree(Shape::right_chain, 100000, 0)};
		test_big("n = 100000, right chain, the middle node and the deepest one", tree, 50000, 99999);
	}
	{
		Tree tree{make_tree(Shape::zigzag, 100000, 0)};
		test_big("n = 100000, zigzag, the deepest node and its far ancestor", tree, 99999, 12345);
	}
	{
		Tree tree{make_tree(Shape::random, 100000, 1)};
		test_big("n = 100000, random tree, the deepest node and the last node", tree, deepest(tree), 99999);
	}
	{
		Tree tree{make_tree(Shape::random, 100000, 2)};
		test_big("n = 100000, another random tree, two arbitrary nodes", tree, 77777, 54321);
	}

	return 0;
}
```

Проверка по времени (1 секунда) не умеет прерывать зависшую программу: если решение зациклится, остановите его сами. Перед отправкой скомпилируйте и прогоните, в том числе под `-fsanitize=address,undefined`.

Пришлите решение вложением. Когда будет готово, я его разберу и закончу списком «в журнал ошибок».

---

17:36 - 18:09

---

```
      1           // {1}
    /   \
  [2]     3       // {1, 2} +1
 / \     /  \
4   [5] 6    7   //  {1, 2} +2 



      1            // {1}
    /   \
   2     [3]       // {1, 2, 3} +1
 / \     / \
4   5   [6] 7      // {1, 2, [3], 4, 5, [6]} +2

      1            // {1}
    /   \
   2     [3]       // {1, 2, [3]}
 / \     / \
4   [5] 6   7      //

      1            //  [0] [1, 2]  [3, 4, 5, 6]
    /   \
  [2]    [3]       //
 / \     / \
4   5   6   7      //

```


---

```cpp
#include <iostream>
#include <format>
#include <algorithm>
#include <cstddef>
#include <vector>
#include <string>
#include <tuple>
#include <random>
#include <chrono>

namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	const TreeNode* find(const TreeNode* node,
					     const TreeNode* p,
					     const TreeNode* q) {
		
		if (node == nullptr || node == p || node == q) { return node; }
		
		const TreeNode* left{find(node->left, p, q)};
		const TreeNode* right{find(node->right, p, q)};
		
		if (left != nullptr && right != nullptr) return node;

		return left != nullptr ? left : right;
	}

	const TreeNode* lowest_common_ancestor(const TreeNode* root,
										   const TreeNode* p,
										   const TreeNode* q) {
		return find(root, p, q);
	}

	// builds a tree from a level-order list, `nil` marks a missing child;
	// the nodes live in `pool`, which must not be reallocated afterwards
	TreeNode* build_tree(const std::vector<int>& level, std::vector<TreeNode>& pool) {
		if (level.empty() || level[0] == nil) {
			return nullptr;
		}
		pool.reserve(level.size());
		pool.push_back(TreeNode{level[0], nullptr, nullptr});
		std::queue<TreeNode*> parents;
		parents.push(&pool.back());
		std::size_t next{1};
		while (!parents.empty() && next < level.size()) {
			TreeNode* parent{parents.front()};
			parents.pop();
			if (next < level.size() && level[next] != nil) {
				pool.push_back(TreeNode{level[next], nullptr, nullptr});
				parent->left = &pool.back();
				parents.push(parent->left);
			}
			++next;
			if (next < level.size() && level[next] != nil) {
				pool.push_back(TreeNode{level[next], nullptr, nullptr});
				parent->right = &pool.back();
				parents.push(parent->right);
			}
			++next;
		}
		return &pool.front();
	}

	const TreeNode* find_by_value(const std::vector<TreeNode>& pool, int value) {
		for (const TreeNode& node : pool) {
			if (node.val == value) {
				return &node;
			}
		}
		return nullptr;
	}

	// the values in `level` are distinct, p and q are values of two different nodes
	void test_lca(const std::string& label,
				  const std::vector<int>& level,
				  int p,
				  int q,
				  int expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		const TreeNode* result{lowest_common_ancestor(root, find_by_value(pool, p), find_by_value(pool, q))};
		std::cout << std::format("{}: {}\n",
			label,
			result == find_by_value(pool, expected) ? "OK" : "FAIL") << std::flush;
	}

	// a tree whose nodes are numbered 0..n-1, the node 0 is the root, the value of a node is its number;
	// the parent of a node always has a smaller number than the node
	struct Tree {
		std::vector<TreeNode> pool;
		std::vector<std::size_t> parent;
		std::vector<std::size_t> depth;
	};

	// reference answer for large trees: climb from the deeper node to the same depth, then both together
	std::size_t expected_lca(const Tree& tree, std::size_t a, std::size_t b) {
		while (tree.depth[a] > tree.depth[b]) a = tree.parent[a];
		while (tree.depth[b] > tree.depth[a]) b = tree.parent[b];
		while (a != b) {
			a = tree.parent[a];
			b = tree.parent[b];
		}
		return a;
	}

	void test_big(const std::string& label, Tree& tree, std::size_t p, std::size_t q) {
		const TreeNode* expected{&tree.pool[expected_lca(tree, p, q)]};

		const auto start{std::chrono::steady_clock::now()};
		const TreeNode* result{lowest_common_ancestor(&tree.pool.front(), &tree.pool[p], &tree.pool[q])};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	Tree make_tree(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		Tree tree{std::vector<TreeNode>(count), std::vector<std::size_t>(count, 0), std::vector<std::size_t>(count, 0)};
		for (std::size_t i{0}; i < count; ++i) {
			tree.pool[i].val = static_cast<int>(i);
		}
		std::mt19937 gen{seed};
		std::vector<std::pair<std::size_t, bool>> free_places{{0, true}, {0, false}};
		for (std::size_t i{1}; i < count; ++i) {
			std::size_t parent{0};
			bool is_left{true};
			switch (shape) {
				case Shape::complete:
					parent = (i - 1) / 2;
					is_left = i % 2 == 1;
					break;
				case Shape::left_chain:
					parent = i - 1;
					break;
				case Shape::right_chain:
					parent = i - 1;
					is_left = false;
					break;
				case Shape::zigzag:
					parent = i - 1;
					is_left = i % 2 == 1;
					break;
				case Shape::random: {
					const std::size_t pick{gen() % free_places.size()};
					std::tie(parent, is_left) = free_places[pick];
					free_places[pick] = free_places.back();
					free_places.pop_back();
					free_places.push_back({i, true});
					free_places.push_back({i, false});
					break;
				}
			}
			(is_left ? tree.pool[parent].left : tree.pool[parent].right) = &tree.pool[i];
			tree.parent[i] = parent;
			tree.depth[i] = tree.depth[parent] + 1;
		}
		return tree;
	}

	// the node with the largest depth
	std::size_t deepest(const Tree& tree) {
		return static_cast<std::size_t>(std::max_element(tree.depth.begin(), tree.depth.end()) - tree.depth.begin());
	}
}

int main() {

	test_lca("description example 1, p and q are in different subtrees",
		{3, 5, 1, 6, 2, 0, 8, nil, nil, 7, 4},
		5, 1, 3);
	test_lca("description example 2, p is an ancestor of q",
		{3, 5, 1, 6, 2, 0, 8, nil, nil, 7, 4},
		5, 4, 5);
	test_lca("two nodes, the root is p",
		{1, 2},
		1, 2, 1);
	test_lca("two nodes, the root is q",
		{1, nil, 2},
		2, 1, 1);
	test_lca("root with two leaves",
		{1, 2, 3},
		2, 3, 1);
	test_lca("root with two leaves, reversed arguments",
		{1, 2, 3},
		3, 2, 1);
	test_lca("siblings deep inside the left subtree",
		{1, 2, 3, 4, 5, nil, nil, 6, 7},
		6, 7, 4);
	test_lca("siblings deep inside the right subtree",
		{1, 2, 3, nil, nil, 4, 5, nil, nil, 6, 7},
		6, 7, 5);
	test_lca("cousins under different parents",
		{1, 2, 3, 4, 5, 6, 7},
		4, 6, 1);
	test_lca("cousins under the same grandparent",
		{1, 2, 3, 4, 5, 6, 7},
		4, 5, 2);
	test_lca("both nodes in the left subtree, the ancestor is below the root",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		8, 11, 2);
	test_lca("both nodes in the right subtree, the ancestor is below the root",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		12, 15, 3);
	test_lca("the nodes are at different depths, p is deeper",
		{1, 2, 3, 4, 5, 6, 7, 8, 9},
		9, 3, 1);
	test_lca("the nodes are at different depths, q is deeper",
		{1, 2, 3, 4, 5, 6, 7, 8, 9},
		3, 9, 1);
	test_lca("p is the parent of q",
		{1, 2, 3, 4, 5, 6, 7},
		2, 5, 2);
	test_lca("q is the parent of p",
		{1, 2, 3, 4, 5, 6, 7},
		5, 2, 2);
	test_lca("p is the grandparent of q",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		3, 14, 3);
	test_lca("p is the root, q is deep",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		1, 15, 1);
	test_lca("q is the root, p is deep",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		15, 1, 1);
	test_lca("left chain, p and q are the deepest nodes",
		{1, 2, nil, 3, nil, 4, nil, 5},
		4, 5, 4);
	test_lca("left chain, p is the top, q is the bottom",
		{1, 2, nil, 3, nil, 4, nil, 5},
		2, 5, 2);
	test_lca("right chain, p is in the middle",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		3, 5, 3);
	test_lca("zigzag, the answer is in the middle",
		{10, 5, nil, nil, 8, 7},
		7, 8, 8);
	test_lca("zigzag, the nodes are the first and the last",
		{10, 5, nil, nil, 8, nil, 11},
		5, 11, 5);
	test_lca("the value zero is among the nodes",
		{0, -1, 1},
		-1, 1, 0);
	test_lca("the value zero is the answer",
		{5, 0, nil, -3, 7},
		-3, 7, 0);
	test_lca("negative values, the answer is the left child",
		{-1, -2, -3, -4, -5},
		-4, -5, -2);
	test_lca("a node with a single child is the answer",
		{1, 2, 3, nil, 4, 5, nil, nil, 6},
		4, 6, 4);
	test_lca("the second subtree is much deeper",
		{1, 2, 3, nil, nil, 4, nil, nil, 5, nil, 6, nil, 7},
		4, 7, 4);
	test_lca("a leaf and the sibling of its ancestor",
		{1, 2, 3, 4, 5, nil, nil, 6},
		6, 3, 1);
	test_lca("random 1, n = 3",
		{72, 26, -6},
		-6, 72, 72);
	test_lca("random 2, n = 5",
		{92, nil, 49, -23, nil, 44, nil, -60},
		49, -60, 49);
	test_lca("random 3, n = 8",
		{-46, 31, -64, nil, nil, 1, -76, nil, 77, nil, -88, nil, nil, -87},
		-88, 31, -46);
	test_lca("random 4, n = 12",
		{16, -68, -38, nil, 21, -98, -58, nil, nil, 72, 4, nil, -57, 42, 25, nil, nil, nil, nil, nil, nil, -16},
		21, -57, 16);
	test_lca("random 5, n = 20",
		{4, 45, 96, nil, -13, nil, 75, 83, -87, 55, -21, 67, nil, 46, -54, nil, 99, nil, 16, nil, nil, nil, 93, nil, -3, nil, 38, -69, 89, nil, nil, nil, nil, nil, nil, nil, nil, nil, -77},
		4, 46, 4);
	test_lca("random 6, n = 30",
		{-46, 27, -30, nil, nil, 100, 86, -44, -3, -99, -58, nil, -49, -61, 65, 2, nil, -85, nil, -43, -79, -100, nil, 32, nil, 50, nil, nil, nil, 35, nil, 12, 8, -35, 66, nil, nil, 62, nil, nil, nil, 74, 61, -45, nil, nil, nil, 48, nil, nil, nil, nil, nil, -93},
		-3, 65, -3);
	test_lca("random 7, n = 40",
		{60, -50, 70, -91, -97, -12, -74, -87, -67, -80, 16, -19, 82, -14, 38, -33, nil, nil, -9, nil, nil, nil, nil, 87, -81, nil, -62, 22, nil, nil, -99, -22, -55, nil, nil, nil, nil, -73, nil, nil, nil, nil, nil, nil, 96, 72, -24, nil, -25, nil, nil, nil, nil, nil, nil, -52, nil, 48, nil, nil, -78, -23, -69, 32, nil, nil, nil, 83, nil, -95, nil, -35, nil, -29, nil, nil, 63},
		-24, -78, -24);
	test_lca("random 8, n = 50",
		{-75, 57, 27, -16, -31, nil, 3, nil, 100, -33, 68, -82, -92, nil, 42, 28, 51, -81, -17, -44, -22, nil, -27, nil, nil, -38, -40, 33, 40, nil, nil, 94, nil, -19, nil, -48, 19, nil, nil, 58, -18, 73, -8, -73, 25, nil, nil, nil, nil, -45, nil, nil, nil, nil, -23, -72, 75, nil, nil, nil, -35, nil, -55, nil, nil, -7, -30, -4, nil, nil, nil, nil, nil, 85, nil, 91, nil, nil, -49, nil, nil, nil, nil, -74, 9, nil, nil, nil, 90, 4, nil, nil, nil, nil, nil, nil, nil, -46},
		-35, -23, -75);

	{
		Tree tree{make_tree(Shape::complete, 100000, 0)};
		test_big("n = 100000, complete tree, two last leaves", tree, 99999, 99998);
	}
	{
		Tree tree{make_tree(Shape::complete, 100000, 0)};
		test_big("n = 100000, complete tree, leaves of different subtrees of the root", tree, 99999, 50000);
	}
	{
		Tree tree{make_tree(Shape::complete, 100000, 0)};
		test_big("n = 100000, complete tree, a leaf and its distant ancestor", tree, 99999, 3);
	}
	{
		Tree tree{make_tree(Shape::left_chain, 100000, 0)};
		test_big("n = 100000, left chain, the deepest node and the middle one", tree, 99999, 50000);
	}
	{
		Tree tree{make_tree(Shape::left_chain, 100000, 0)};
		test_big("n = 100000, left chain, the deepest node and the root", tree, 99999, 0);
	}
	{
		Tree tree{make_tree(Shape::left_chain, 100000, 0)};
		test_big("n = 100000, left chain, two deepest nodes", tree, 99999, 99998);
	}
	{
		Tree tree{make_tree(Shape::right_chain, 100000, 0)};
		test_big("n = 100000, right chain, the middle node and the deepest one", tree, 50000, 99999);
	}
	{
		Tree tree{make_tree(Shape::zigzag, 100000, 0)};
		test_big("n = 100000, zigzag, the deepest node and its far ancestor", tree, 99999, 12345);
	}
	{
		Tree tree{make_tree(Shape::random, 100000, 1)};
		test_big("n = 100000, random tree, the deepest node and the last node", tree, deepest(tree), 99999);
	}
	{
		Tree tree{make_tree(Shape::random, 100000, 2)};
		test_big("n = 100000, another random tree, two arbitrary nodes", tree, 77777, 54321);
	}

	return 0;
}
```