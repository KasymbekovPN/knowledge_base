
Задача 5 блока 7: Validate Binary Search Tree. Собираю тесты и проверяю на неверных вариантах.

Задача 5 блока 7 готова. Проверка по запуску: заглушка даёт 24 FAIL из 55, два разных эталона (рекурсия с границами и обход in-order) проходят все 55 без предупреждений и чисто под ASan/UBSan, пять неверных вариантов (проверка только родителя, нестрогое сравнение, границы через `INT_MIN`/`INT_MAX`, граница только с одной стороны, `INT_MIN` как начальное «предыдущее») ловятся.

## Validate Binary Search Tree

Дано бинарное дерево. Определите, является ли оно корректным бинарным деревом поиска: у каждого узла все значения в левом поддереве строго меньше его значения, а в правом строго больше (оба поддерева тоже деревья поиска). Пустое дерево корректно.

```cpp
bool is_valid_bst(const TreeNode* root);
```

**Ограничения:** 0 ≤ n ≤ 10⁵, значения лежат во всём диапазоне `int`.

**Вопросы интервьюера:**

1. Почему недостаточно сравнить каждый узел с его детьми? Приведите контрпример.
2. Какие границы вы передаёте вниз и как избегаете проблем с `INT_MIN` и `INT_MAX`?
3. Как обойти это обходом in-order? Что именно проверяется?
4. Сложность по времени и по памяти? Что с рекурсией на цепочке из 10⁵ узлов?

**Цель:** O(n) по времени, O(h) по памяти. **Ориентир по времени:** 12 минут.

```cpp
namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	bool is_valid_bst([[maybe_unused]] const TreeNode* root) {
		return false;
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

	// reference answer: an in-order walk with an explicit stack,
	// a tree is a search tree exactly when the values come out strictly increasing
	bool expected_bst(const TreeNode* root) {
		std::vector<const TreeNode*> stack;
		const TreeNode* node{root};
		bool has_previous{false};
		long long previous{0};
		while (node != nullptr || !stack.empty()) {
			while (node != nullptr) {
				stack.push_back(node);
				node = node->left;
			}
			node = stack.back();
			stack.pop_back();
			if (has_previous && node->val <= previous) {
				return false;
			}
			has_previous = true;
			previous = node->val;
			node = node->right;
		}
		return true;
	}

	void test_bst(const std::string& label,
				  const std::vector<int>& level,
				  bool expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			is_valid_bst(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	// the same, but the values are given separately, in the order of the nodes in `level`
	void test_bst_values(const std::string& label,
						 const std::vector<int>& level,
						 const std::vector<int>& values,
						 bool expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		for (std::size_t i{0}; i < pool.size(); ++i) {
			pool[i].val = values[i];
		}
		std::cout << std::format("{}: {}\n",
			label,
			is_valid_bst(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	void test_big(const std::string& label, const TreeNode* root) {
		const bool expected{expected_bst(root)};

		const auto start{std::chrono::steady_clock::now()};
		const bool result{is_valid_bst(root)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// a search tree of the given shape: the node 0 is the root,
	// the values grow with the in-order position and are spread over the whole int range
	std::vector<TreeNode> make_bst(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		std::vector<TreeNode> pool(count);
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
			(is_left ? pool[parent].left : pool[parent].right) = &pool[i];
		}
		std::vector<TreeNode*> stack;
		TreeNode* node{&pool.front()};
		long long rank{0};
		while (node != nullptr || !stack.empty()) {
			while (node != nullptr) {
				stack.push_back(node);
				node = node->left;
			}
			node = stack.back();
			stack.pop_back();
			node->val = static_cast<int>(std::numeric_limits<int>::min() + rank * 20000);
			++rank;
			node = node->right;
		}
		return pool;
	}
}

int main() {

	test_bst("description example 1, valid",
		{2, 1, 3},
		true);
	test_bst("description example 2, invalid",
		{5, 1, 4, nil, nil, 3, 6},
		false);
	test_bst("empty tree",
		{},
		true);
	test_bst("single node",
		{5},
		true);
	test_bst("left child is bigger",
		{2, 3},
		false);
	test_bst("right child is smaller",
		{2, nil, 1},
		false);
	test_bst("left child is equal",
		{2, 2},
		false);
	test_bst("right child is equal",
		{2, nil, 2},
		false);
	test_bst("equal values everywhere",
		{7, 7, 7},
		false);
	test_bst("perfect tree of 7 nodes, valid",
		{4, 2, 6, 1, 3, 5, 7},
		true);
	test_bst("perfect tree of 7 nodes, left leaf is bigger than the parent",
		{4, 2, 6, 3, 1, 5, 7},
		false);
	test_bst("a node is smaller than its parent but bigger than the root",
		{5, 3, 7, 2, 6, 6, 8},
		false);
	test_bst("right grandchild is smaller than the root",
		{5, 4, 6, nil, nil, 3, 7},
		false);
	test_bst("a node of the left subtree is bigger than the root",
		{5, 1, 8, nil, 6},
		false);
	test_bst("a node of the right subtree is smaller than the root",
		{10, 5, 15, nil, nil, 6, 20},
		false);
	test_bst("a grandchild equals the root",
		{5, 3, 8, nil, 5},
		false);
	test_bst("a deep node equals an ancestor on the far side",
		{5, 3, 8, 1, 4, 6, 8},
		false);
	test_bst("left chain, valid",
		{5, 4, nil, 3, nil, 2, nil, 1},
		true);
	test_bst("left chain, the deepest value is bigger than the root",
		{5, 4, nil, 3, nil, 2, nil, 6},
		false);
	test_bst("right chain, valid",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		true);
	test_bst("right chain, the deepest value repeats the root",
		{1, nil, 2, nil, 3, nil, 4, nil, 1},
		false);
	test_bst("zigzag, valid",
		{10, 5, nil, nil, 8, 7},
		true);
	test_bst("zigzag, the deepest value is bigger than the root",
		{10, 5, nil, nil, 8, nil, 11},
		false);
	test_bst("zigzag, the deepest value is smaller than an ancestor",
		{10, 5, nil, nil, 8, 4},
		false);
	test_bst("negative values, valid",
		{-5, -10, -1, nil, -7, -3, 0},
		true);
	test_bst("negative values, invalid",
		{-5, -10, -1, nil, -3},
		false);
	test_bst("zero in the middle, valid",
		{0, -1, 1},
		true);
	test_bst("zero in the middle, invalid",
		{0, 1, -1},
		false);
	test_bst("valid, the right child of the left child is just below the root",
		{100, -100, nil, nil, 99},
		true);
	test_bst("complete tree of 15 nodes, valid",
		{8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15},
		true);
	test_bst("complete tree of 15 nodes, swapped leaves",
		{8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 15, 13},
		false);
	test_bst("complete tree of 15 nodes, a leaf equals its parent",
		{8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 14},
		false);

	constexpr int lowest{std::numeric_limits<int>::min()};
	constexpr int highest{std::numeric_limits<int>::max()};
	test_bst_values("the lowest int alone", {1}, {lowest}, true);
	test_bst_values("the highest int alone", {1}, {highest}, true);
	test_bst_values("lowest, 0, highest", {1, 1, 1}, {0, lowest, highest}, true);
	test_bst_values("the lowest int is the root, the highest is its right child", {1, nil, 1}, {lowest, highest}, true);
	test_bst_values("the highest int is the root, the lowest is its left child", {1, 1}, {highest, lowest}, true);
	test_bst_values("the lowest int repeats in the left child", {1, 1}, {lowest, lowest}, false);
	test_bst_values("the highest int repeats in the right child", {1, nil, 1}, {highest, highest}, false);
	test_bst_values("the lowest int is the left child of the lowest", {1, 1, 1}, {0, lowest, lowest}, false);
	test_bst_values("the highest int is deep in the left subtree of 0", {1, 1, nil, nil, 1}, {0, lowest, highest}, false);
	test_bst_values("the lowest int is deep in the right subtree of 0", {1, nil, 1, 1}, {0, highest, lowest}, false);
	test_bst_values("a chain from the highest down to the lowest", {1, 1, nil, 1, nil, 1}, {highest, 1, 0, lowest}, true);
	test_bst_values("a chain from the lowest up to the highest", {1, nil, 1, nil, 1, nil, 1}, {lowest, -1, 0, highest}, true);

	{
		std::vector<TreeNode> pool{make_bst(Shape::complete, 100000, 0)};
		test_big("n = 100000, complete tree, valid", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::complete, 100000, 0)};
		std::swap(pool.front().val, pool.back().val);
		test_big("n = 100000, complete tree, the root and the last leaf are swapped", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::left_chain, 100000, 0)};
		test_big("n = 100000, left chain, valid", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::left_chain, 100000, 0)};
		pool.back().val = pool.front().val;
		test_big("n = 100000, left chain, the deepest value equals the root", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::right_chain, 100000, 0)};
		test_big("n = 100000, right chain, valid", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::right_chain, 100000, 0)};
		pool.back().val = pool.front().val;
		test_big("n = 100000, right chain, the deepest value equals the root", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::zigzag, 100000, 0)};
		test_big("n = 100000, zigzag, valid", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::zigzag, 100000, 0)};
		pool.back().val = pool.front().val;
		test_big("n = 100000, zigzag, the deepest value equals the root", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::random, 100000, 1)};
		test_big("n = 100000, random tree, valid", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::random, 100000, 2)};
		std::swap(pool.front().val, pool.back().val);
		test_big("n = 100000, random tree, the root and the last node are swapped", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::random, 100000, 3)};
		pool.back().val += 1;
		test_big("n = 100000, random tree, the last value is shifted by 1", &pool.front());
	}

	return 0;
}
```

Проверка по времени (1 секунда) не умеет прерывать зависшую программу: если решение зациклится, остановите его сами. Перед отправкой скомпилируйте и прогоните, в том числе под `-fsanitize=address,undefined`.

Пришлите решение вложением. Когда будет готово, я его разберу и закончу списком «в журнал ошибок».

---

16:03  - 16:33

---

// +
```
        8            // {8}
      /   \
     4     12        // 4 < 8 && 8 <= 12 =>  {8, 4, 12}
    / \   /  \
   2   6 10   14
```

// -
```
      5
     / \
    4   6
       / \
      3   7
```

// -
```
    2
   /
  2
```

---


```cpp
#include <iostream>
#include <format>
#include <algorithm>
#include <limits>
#include <vector>
#include <tuple>
#include <string>
#include <deque>
#include <climits>
#include <cstddef>
#include <chrono>
#include <random>

namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	bool is_valid(const TreeNode* node, const TreeNode* lo, const TreeNode* hi) {
		if (!node) return true;
		if (lo && node->val <= lo->val) return false;
		if (hi && node->val >= hi->val) return false;
		
		return
			is_valid(node->left, lo, node) &&
			is_valid(node->right, node, hi);
	}
}

	bool is_valid_bst(const TreeNode* root) {
		return is_valid(root, nullptr, nullptr);
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

	// reference answer: an in-order walk with an explicit stack,
	// a tree is a search tree exactly when the values come out strictly increasing
	bool expected_bst(const TreeNode* root) {
		std::vector<const TreeNode*> stack;
		const TreeNode* node{root};
		bool has_previous{false};
		long long previous{0};
		while (node != nullptr || !stack.empty()) {
			while (node != nullptr) {
				stack.push_back(node);
				node = node->left;
			}
			node = stack.back();
			stack.pop_back();
			if (has_previous && node->val <= previous) {
				return false;
			}
			has_previous = true;
			previous = node->val;
			node = node->right;
		}
		return true;
	}

	void test_bst(const std::string& label,
				  const std::vector<int>& level,
				  bool expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			is_valid_bst(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	// the same, but the values are given separately, in the order of the nodes in `level`
	void test_bst_values(const std::string& label,
						 const std::vector<int>& level,
						 const std::vector<int>& values,
						 bool expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		for (std::size_t i{0}; i < pool.size(); ++i) {
			pool[i].val = values[i];
		}
		std::cout << std::format("{}: {}\n",
			label,
			is_valid_bst(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	void test_big(const std::string& label, const TreeNode* root) {
		const bool expected{expected_bst(root)};

		const auto start{std::chrono::steady_clock::now()};
		const bool result{is_valid_bst(root)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// a search tree of the given shape: the node 0 is the root,
	// the values grow with the in-order position and are spread over the whole int range
	std::vector<TreeNode> make_bst(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		std::vector<TreeNode> pool(count);
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
			(is_left ? pool[parent].left : pool[parent].right) = &pool[i];
		}
		std::vector<TreeNode*> stack;
		TreeNode* node{&pool.front()};
		long long rank{0};
		while (node != nullptr || !stack.empty()) {
			while (node != nullptr) {
				stack.push_back(node);
				node = node->left;
			}
			node = stack.back();
			stack.pop_back();
			node->val = static_cast<int>(std::numeric_limits<int>::min() + rank * 20000);
			++rank;
			node = node->right;
		}
		return pool;
	}
}

int main() {

	test_bst("description example 1, valid",
		{2, 1, 3},
		true);
	test_bst("description example 2, invalid",
		{5, 1, 4, nil, nil, 3, 6},
		false);
	test_bst("empty tree",
		{},
		true);
	test_bst("single node",
		{5},
		true);
	test_bst("left child is bigger",
		{2, 3},
		false);
	test_bst("right child is smaller",
		{2, nil, 1},
		false);
	test_bst("left child is equal",
		{2, 2},
		false);
	test_bst("right child is equal",
		{2, nil, 2},
		false);
	test_bst("equal values everywhere",
		{7, 7, 7},
		false);
	test_bst("perfect tree of 7 nodes, valid",
		{4, 2, 6, 1, 3, 5, 7},
		true);
	test_bst("perfect tree of 7 nodes, left leaf is bigger than the parent",
		{4, 2, 6, 3, 1, 5, 7},
		false);
	test_bst("a node is smaller than its parent but bigger than the root",
		{5, 3, 7, 2, 6, 6, 8},
		false);
	test_bst("right grandchild is smaller than the root",
		{5, 4, 6, nil, nil, 3, 7},
		false);
	test_bst("a node of the left subtree is bigger than the root",
		{5, 1, 8, nil, 6},
		false);
	test_bst("a node of the right subtree is smaller than the root",
		{10, 5, 15, nil, nil, 6, 20},
		false);
	test_bst("a grandchild equals the root",
		{5, 3, 8, nil, 5},
		false);
	test_bst("a deep node equals an ancestor on the far side",
		{5, 3, 8, 1, 4, 6, 8},
		false);
	test_bst("left chain, valid",
		{5, 4, nil, 3, nil, 2, nil, 1},
		true);
	test_bst("left chain, the deepest value is bigger than the root",
		{5, 4, nil, 3, nil, 2, nil, 6},
		false);
	test_bst("right chain, valid",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		true);
	test_bst("right chain, the deepest value repeats the root",
		{1, nil, 2, nil, 3, nil, 4, nil, 1},
		false);
	test_bst("zigzag, valid",
		{10, 5, nil, nil, 8, 7},
		true);
	test_bst("zigzag, the deepest value is bigger than the root",
		{10, 5, nil, nil, 8, nil, 11},
		false);
	test_bst("zigzag, the deepest value is smaller than an ancestor",
		{10, 5, nil, nil, 8, 4},
		false);
	test_bst("negative values, valid",
		{-5, -10, -1, nil, -7, -3, 0},
		true);
	test_bst("negative values, invalid",
		{-5, -10, -1, nil, -3},
		false);
	test_bst("zero in the middle, valid",
		{0, -1, 1},
		true);
	test_bst("zero in the middle, invalid",
		{0, 1, -1},
		false);
	test_bst("valid, the right child of the left child is just below the root",
		{100, -100, nil, nil, 99},
		true);
	test_bst("complete tree of 15 nodes, valid",
		{8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15},
		true);
	test_bst("complete tree of 15 nodes, swapped leaves",
		{8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 15, 13},
		false);
	test_bst("complete tree of 15 nodes, a leaf equals its parent",
		{8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 14},
		false);

	constexpr int lowest{std::numeric_limits<int>::min()};
	constexpr int highest{std::numeric_limits<int>::max()};
	test_bst_values("the lowest int alone", {1}, {lowest}, true);
	test_bst_values("the highest int alone", {1}, {highest}, true);
	test_bst_values("lowest, 0, highest", {1, 1, 1}, {0, lowest, highest}, true);
	test_bst_values("the lowest int is the root, the highest is its right child", {1, nil, 1}, {lowest, highest}, true);
	test_bst_values("the highest int is the root, the lowest is its left child", {1, 1}, {highest, lowest}, true);
	test_bst_values("the lowest int repeats in the left child", {1, 1}, {lowest, lowest}, false);
	test_bst_values("the highest int repeats in the right child", {1, nil, 1}, {highest, highest}, false);
	test_bst_values("the lowest int is the left child of the lowest", {1, 1, 1}, {0, lowest, lowest}, false);
	test_bst_values("the highest int is deep in the left subtree of 0", {1, 1, nil, nil, 1}, {0, lowest, highest}, false);
	test_bst_values("the lowest int is deep in the right subtree of 0", {1, nil, 1, 1}, {0, highest, lowest}, false);
	test_bst_values("a chain from the highest down to the lowest", {1, 1, nil, 1, nil, 1}, {highest, 1, 0, lowest}, true);
	test_bst_values("a chain from the lowest up to the highest", {1, nil, 1, nil, 1, nil, 1}, {lowest, -1, 0, highest}, true);

	{
		std::vector<TreeNode> pool{make_bst(Shape::complete, 100000, 0)};
		test_big("n = 100000, complete tree, valid", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::complete, 100000, 0)};
		std::swap(pool.front().val, pool.back().val);
		test_big("n = 100000, complete tree, the root and the last leaf are swapped", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::left_chain, 100000, 0)};
		test_big("n = 100000, left chain, valid", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::left_chain, 100000, 0)};
		pool.back().val = pool.front().val;
		test_big("n = 100000, left chain, the deepest value equals the root", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::right_chain, 100000, 0)};
		test_big("n = 100000, right chain, valid", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::right_chain, 100000, 0)};
		pool.back().val = pool.front().val;
		test_big("n = 100000, right chain, the deepest value equals the root", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::zigzag, 100000, 0)};
		test_big("n = 100000, zigzag, valid", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::zigzag, 100000, 0)};
		pool.back().val = pool.front().val;
		test_big("n = 100000, zigzag, the deepest value equals the root", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::random, 100000, 1)};
		test_big("n = 100000, random tree, valid", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::random, 100000, 2)};
		std::swap(pool.front().val, pool.back().val);
		test_big("n = 100000, random tree, the root and the last node are swapped", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_bst(Shape::random, 100000, 3)};
		pool.back().val += 1;
		test_big("n = 100000, random tree, the last value is shifted by 1", &pool.front());
	}

	return 0;
}
```