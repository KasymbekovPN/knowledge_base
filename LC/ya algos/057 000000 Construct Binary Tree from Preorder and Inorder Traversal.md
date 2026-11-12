Задача 9 блока 7: Construct Binary Tree from Preorder and Inorder Traversal. Собираю тесты и проверяю.

Задача 9 блока 7 готова, собираю итоговое сообщение. Проверка по запуску: заглушка (возвращает `nullptr`) собирается без единого предупреждения и даёт 34 FAIL из 35; два разных эталона (рекурсия с отображением позиций и итеративный со стеком) проходят все 35 и чисто под ASan/UBSan; вариант с линейным поиском корня в `inorder` корректен, но на цепочке из 10⁵ узлов помечается `(too slow)`; три неверных варианта (дети переставлены местами, сдвиг границы правого поддерева на единицу, всё в левую цепочку) ловятся.

## Construct Binary Tree from Preorder and Inorder Traversal

Даны два обхода одного и того же бинарного дерева с **различными** значениями: прямой (`preorder`: узел, левое, правое) и симметричный (`inorder`: левое, узел, правое). Постройте это дерево и верните его корень. Для пустых обходов верните `nullptr`.

Узлы вы создаёте в переданном `pool`, чей запас (`capacity`) уже равен `preorder.size()`: добавляйте узлы через `pool.push_back(...)` и никогда не превышайте это число, иначе вектор переедет и указатели станут недействительными.

```cpp
TreeNode* build_from_traversals(const std::vector<int>& preorder,
                                const std::vector<int>& inorder,
                                std::vector<TreeNode>& pool);
```

**Ограничения:** 0 ≤ n ≤ 10⁵, значения различны (во всём диапазоне `int`), обходы корректны и относятся к одному дереву.

**Вопросы интервьюера:**

1. Как по двум обходам однозначно восстанавливается дерево? Что даёт `preorder`, а что `inorder`?
2. Почему нельзя обойтись одним обходом? Приведите два разных дерева с одинаковым `preorder`.
3. Как искать позицию корня в `inorder` и сколько это стоит? Что произойдёт при поиске линейным проходом на цепочке из 10⁵ узлов?
4. Какая сложность по времени и по памяти? Что с рекурсией на цепочке и как обойтись без неё?

**Цель:** O(n) по времени (предварительно запомнить позиции значений из `inorder`). **Ориентир по времени:** 20 минут.

```cpp
namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	// the nodes of the result must be created in `pool`, whose capacity is already preorder.size(),
	// so the pool must never be reallocated
	TreeNode* build_from_traversals([[maybe_unused]] const std::vector<int>& preorder,
									[[maybe_unused]] const std::vector<int>& inorder,
									[[maybe_unused]] std::vector<TreeNode>& pool) {
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

	// the trees are equal when they have the same shape and the same values
	bool same_tree(const TreeNode* a, const TreeNode* b) {
		std::vector<std::pair<const TreeNode*, const TreeNode*>> stack{{a, b}};
		while (!stack.empty()) {
			const auto [x, y]{stack.back()};
			stack.pop_back();
			if (x == nullptr || y == nullptr) {
				if (x != y) return false;
				continue;
			}
			if (x->val != y->val) return false;
			stack.push_back({x->left, y->left});
			stack.push_back({x->right, y->right});
		}
		return true;
	}

	void test_build(const std::string& label,
					const std::vector<int>& preorder,
					const std::vector<int>& inorder,
					const std::vector<int>& expected) {
		std::vector<TreeNode> expected_pool;
		const TreeNode* expected_root{build_tree(expected, expected_pool)};

		std::vector<TreeNode> pool;
		pool.reserve(preorder.size());
		const TreeNode* root{build_from_traversals(preorder, inorder, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			same_tree(root, expected_root) ? "OK" : "FAIL") << std::flush;
	}

	// both traversals of a tree, computed without recursion
	void make_traversals(const TreeNode* root, std::vector<int>& preorder, std::vector<int>& inorder) {
		std::vector<const TreeNode*> stack;
		const TreeNode* node{root};
		while (node != nullptr || !stack.empty()) {
			while (node != nullptr) {
				preorder.push_back(node->val);
				stack.push_back(node);
				node = node->left;
			}
			node = stack.back();
			stack.pop_back();
			inorder.push_back(node->val);
			node = node->right;
		}
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// a tree of the given shape, the node 0 is the root; the values are distinct
	std::vector<TreeNode> make_tree(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		std::vector<TreeNode> pool(count);
		for (std::size_t i{0}; i < count; ++i) {
			pool[i].val = static_cast<int>((i * 7919) % 100003) - 50000;
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
			(is_left ? pool[parent].left : pool[parent].right) = &pool[i];
		}
		return pool;
	}

	void test_big(const std::string& label, Shape shape, int n, unsigned seed) {
		const std::vector<TreeNode> original{make_tree(shape, n, seed)};
		std::vector<int> preorder;
		std::vector<int> inorder;
		make_traversals(&original.front(), preorder, inorder);

		std::vector<TreeNode> pool;
		pool.reserve(preorder.size());
		const auto start{std::chrono::steady_clock::now()};
		const TreeNode* root{build_from_traversals(preorder, inorder, pool)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			same_tree(root, &original.front()) ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}
}

int main() {

	test_build("description example 1",
		{3, 9, 20, 15, 7},
		{9, 3, 15, 20, 7},
		{3, 9, 20, nil, nil, 15, 7});
	test_build("description example 2, single node",
		{-1},
		{-1},
		{-1});
	test_build("empty traversals",
		{},
		{},
		{});
	test_build("two nodes, the left child",
		{1, 2},
		{2, 1},
		{1, 2});
	test_build("two nodes, the right child",
		{1, 2},
		{1, 2},
		{1, nil, 2});
	test_build("root with two leaves",
		{2, 1, 3},
		{1, 2, 3},
		{2, 1, 3});
	test_build("left chain of 5, the preorder is the reversed inorder",
		{5, 4, 3, 2, 1},
		{1, 2, 3, 4, 5},
		{5, 4, nil, 3, nil, 2, nil, 1});
	test_build("right chain of 5, the preorder equals the inorder",
		{1, 2, 3, 4, 5},
		{1, 2, 3, 4, 5},
		{1, nil, 2, nil, 3, nil, 4, nil, 5});
	test_build("zigzag of 5",
		{1, 2, 3, 4, 5},
		{2, 4, 5, 3, 1},
		{1, 2, nil, nil, 3, 4, nil, nil, 5});
	test_build("zigzag of 5, starts to the right",
		{1, 2, 3, 4, 5},
		{1, 3, 4, 5, 2},
		{1, nil, 2, 3, nil, nil, 4, nil, 5});
	test_build("complete tree of 7 nodes",
		{4, 2, 1, 3, 6, 5, 7},
		{1, 2, 3, 4, 5, 6, 7},
		{4, 2, 6, 1, 3, 5, 7});
	test_build("complete tree of 15 nodes",
		{8, 4, 2, 1, 3, 6, 5, 7, 12, 10, 9, 11, 14, 13, 15},
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		{8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15});
	test_build("values are not sorted",
		{9, 4, 8, 1, 7, 2, 6},
		{8, 4, 1, 9, 2, 7, 6},
		{9, 4, 7, 8, 1, 2, 6});
	test_build("negative values",
		{-5, -8, -9, -6, -2, -3, -1},
		{-9, -8, -6, -5, -3, -2, -1},
		{-5, -8, -2, -9, -6, -3, -1});
	test_build("only the left child has children",
		{1, 2, 4, 6, 7, 5, 3},
		{6, 4, 7, 2, 5, 1, 3},
		{1, 2, 3, 4, 5, nil, nil, 6, 7});
	test_build("only the right child has children",
		{1, 2, 3, 4, 5, 6, 7},
		{2, 1, 4, 3, 6, 5, 7},
		{1, 2, 3, nil, nil, 4, 5, nil, nil, 6, 7});
	test_build("the left subtree is much deeper",
		{1, 2, 4, 5, 6, 3},
		{6, 5, 4, 2, 1, 3},
		{1, 2, 3, 4, nil, nil, nil, 5, nil, 6});
	test_build("the right subtree is much deeper",
		{1, 2, 3, 4, 5, 6},
		{2, 1, 3, 4, 5, 6},
		{1, 2, 3, nil, nil, nil, 4, nil, 5, nil, 6});
	test_build("a node with a single right child inside the left subtree",
		{1, 2, 4, 3, 5, 6},
		{2, 4, 1, 6, 5, 3},
		{1, 2, 3, nil, 4, 5, nil, nil, nil, 6});
	test_build("a node with a single left child inside the right subtree",
		{1, 2, 4, 3, 5, 6},
		{4, 2, 1, 3, 6, 5},
		{1, 2, 3, 4, nil, nil, 5, nil, nil, 6});
	test_build("zero among the values",
		{0, -1, -2, 1, 2},
		{-2, -1, 0, 1, 2},
		{0, -1, 1, -2, nil, nil, 2});
	test_build("random 1, n = 3",
		{51, -11, 48},
		{-11, 51, 48},
		{51, -11, 48});
	test_build("random 2, n = 5",
		{-21, 96, 46, 34, -15},
		{46, 96, -21, 34, -15},
		{-21, 96, 34, 46, nil, nil, -15});
	test_build("random 3, n = 8",
		{92, -63, -75, -57, 68, 1, -11, 10},
		{92, -57, -75, 68, -63, 1, 10, -11},
		{92, nil, -63, -75, 1, -57, 68, nil, -11, nil, nil, nil, nil, 10});
	test_build("random 4, n = 12",
		{34, -36, 72, -53, -85, -17, -9, 59, -71, 85, 88, 95},
		{34, -53, 72, -36, -85, -9, -17, 88, 85, 95, -71, 59},
		{34, nil, -36, 72, -85, -53, nil, nil, -17, nil, nil, -9, 59, nil, nil, -71, nil, 85, nil, 88, 95});
	test_build("random 5, n = 20",
		{-9, 23, -26, 80, 41, 40, 96, -53, -33, 44, -68, 48, -52, 56, -72, 100, 0, -48, -97, -86},
		{23, -9, -26, 40, 41, 96, -53, 80, 56, -52, 48, -68, 44, -72, -33, 100, -48, 0, -97, -86},
		{-9, 23, -26, nil, nil, nil, 80, 41, -33, 40, 96, 44, 100, nil, nil, nil, -53, -68, -72, nil, 0, nil, nil, 48, nil, nil, nil, -48, -97, -52, nil, nil, nil, nil, -86, 56});
	test_build("random 6, n = 30",
		{-79, -72, 25, -73, -58, 11, 59, -38, 79, -29, 35, -89, -27, -45, 86, -7, -46, 75, 36, -53, 20, -13, -78, -70, 58, -31, -98, -40, 24, 32},
		{25, -72, -38, 79, 59, -29, 11, 35, -58, -27, 86, -45, -46, -7, 75, 36, -89, -73, -53, 20, -79, -31, 58, -70, -78, -98, -13, 24, -40, 32},
		{-79, -72, -13, 25, -73, -78, -40, nil, nil, -58, -53, -70, -98, 24, 32, 11, -89, nil, 20, 58, nil, nil, nil, nil, nil, nil, nil, 59, 35, -27, nil, nil, nil, -31, nil, -38, -29, nil, nil, nil, -45, nil, nil, nil, 79, nil, nil, 86, -7, nil, nil, nil, nil, -46, 75, nil, nil, nil, 36});
	test_build("random 7, n = 40",
		{99, -92, -66, -93, 43, -78, -56, -55, -94, -37, -8, 53, 90, -23, 67, 45, 21, -53, -77, -20, 13, 86, 69, -11, 89, 74, 73, -4, -46, 22, -100, 61, -82, -91, 15, -47, 95, 75, 20, 6},
		{-93, -66, -92, -55, -56, -37, 90, 53, -8, -94, -23, -78, 45, 67, -53, 21, 43, -20, -77, 86, 13, 99, -11, 89, 74, -4, 73, 69, 22, -82, 61, -100, -46, 95, -47, 15, -91, 75, 20, 6},
		{99, -92, 69, -66, 43, -11, -46, -93, nil, -78, -77, nil, 89, 22, -91, nil, nil, -56, 67, -20, 13, nil, 74, nil, -100, 15, 75, -55, -94, 45, 21, nil, nil, 86, nil, nil, 73, 61, nil, -47, nil, nil, 20, nil, nil, -37, -23, nil, nil, -53, nil, nil, nil, -4, nil, -82, nil, 95, nil, nil, 6, nil, -8, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 53, nil, 90});
	test_build("random 8, n = 50",
		{-33, 79, -14, 4, 82, 97, -17, 98, -35, -84, -9, 89, -53, 20, -47, -4, -25, 1, -37, 60, 90, 9, -67, 7, -23, 22, 78, 92, 80, -81, 51, 88, -90, 14, 17, 50, 8, 21, 96, 11, 71, -3, -30, -15, -94, -18, -97, 72, 34, 56},
		{4, 97, -17, 82, -14, -35, -84, 98, 79, -53, 89, -9, 20, -33, 90, 9, 60, -67, -37, 1, 22, 78, -23, 80, 92, 7, -81, 51, 14, -90, 88, -25, 17, -4, 96, 21, 71, 11, 8, 50, -3, -30, -47, -18, -94, -97, 72, -15, 34, 56},
		{-33, 79, -47, -14, -9, -4, -15, 4, 98, 89, 20, -25, 50, -94, 34, nil, 82, -35, nil, -53, nil, nil, nil, 1, 17, 8, -3, -18, -97, nil, 56, 97, nil, nil, -84, nil, nil, -37, 7, nil, nil, 21, nil, nil, -30, nil, nil, nil, 72, nil, nil, nil, -17, nil, nil, 60, nil, -23, -81, 96, 11, nil, nil, nil, nil, nil, nil, 90, -67, 22, 92, nil, 51, nil, nil, 71, nil, nil, 9, nil, nil, nil, 78, 80, nil, nil, 88, nil, nil, nil, nil, nil, nil, nil, nil, -90, nil, 14});

	test_big("n = 100000, complete tree", Shape::complete, 100000, 0);
	test_big("n = 100000, left chain", Shape::left_chain, 100000, 0);
	test_big("n = 100000, right chain", Shape::right_chain, 100000, 0);
	test_big("n = 100000, zigzag", Shape::zigzag, 100000, 0);
	test_big("n = 100000, random tree", Shape::random, 100000, 1);
	test_big("n = 100000, another random tree", Shape::random, 100000, 2);

	return 0;
}
```

Проверка по времени (1 секунда) не умеет прерывать зависшую программу: если решение зациклится (или поиск корня в `inorder` линейный, как в наивном варианте), остановите его сами. Рекурсия глубиной 10⁵ нормально работает без санитайзеров, но под `-fsanitize=address` может упасть по стеку из-за размера кадров: это не ошибка алгоритма, увеличьте стек через `ulimit -s unlimited` или проверьте итеративный вариант.

Пришлите решение вложением. Когда будет готово, я его разберу и закончу списком «в журнал ошибок».

---

14:56 - 15:39

---

```
        5
       / \
      3   6
     / \
    2   4
   /
  1
```

- **Preorder (прямой):** `5, 3, 2, 1, 4, 6`
- **Inorder (центрированный):** `1, 2, 3, 4, 5, 6`

- {5, 3, 2, 1, 4, 6} & {1, 2, 3, 4, 5, 6}
- {[5], 3, 2, 1, 4, 6} & {(1, 2, 3, 4), [5], (6)}
- to left {5, [3], 2, 1, 4, 6} & {(1, 2), [3], (4)}
- to left {5, [3], 2, 1, 4, 6} & {[4]}
- to left {5, 3, [2], 1, 4, 6} & {(1), [2]}
- to left {5, 3, 2, [1], 4, 6} & {[1]}

---

```cpp
#include <iostream>
#include <format>
#include <utility>
#include <vector>
#include <queue>
#include <tuple>
#include <string>
#include <cstddef>
#include <unordered_map>
#include <random>
#include <chrono>
#include <algorithm>

namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	TreeNode* build(const std::vector<int>& preorder,
					const std::vector<int>& inorder,
					std::vector<TreeNode>& pool,
					int& preorder_idx,
					std::vector<int>::const_iterator begin,
					std::vector<int>::const_iterator end) {

		const int current_value{preorder[preorder_idx]};
		pool.push_back({current_value, nullptr, nullptr});
		TreeNode* node{&pool[preorder_idx++]};
		
		const auto it{std::find_if(begin, end, [current_value](const int value) {
			return current_value == value;
		})};
		
		if (it == inorder.end()) return nullptr;
		
		node->left = build(
			preorder,
			inorder,
			pool,
			preorder_idx,
			begin,
			it
		);
		node->right = build(
			preorder,
			inorder,
			pool,
			preorder_idx,
			it + 1,
			end
		);
		
		return node;
	}

	TreeNode* build(const std::vector<int>& preorder,
                const std::unordered_map<int, std::size_t>& position,
                std::size_t& next,
                std::size_t low,
                std::size_t high,
                std::vector<TreeNode>& pool) {
                
        if (low >= high) return nullptr;
        pool.push_back(TreeNode{[preorder[next], nullptr, nullptr]});
        TreeNode* node{&pool.back()};
        
        const std::size_t mid{position.at(preorder[next])};
	    ++next;
	    
	    node->left = build(preorder, position, next, low, mid, pool);
	    node->right = build(preorder, position, next, mid + 1, high, pool);
	    
	    return node;
	}

	// the nodes of the result must be created in `pool`, whose capacity is already preorder.size(),
	// so the pool must never be reallocated
	TreeNode* build_from_traversals(const std::vector<int>& preorder,
									const std::vector<int>& inorder,
									std::vector<TreeNode>& pool) {
		const std::size_t size{preorder.size()};
		if (size == 0) return nullptr;

		std::size_t next{0};
		std::unordered_map<int, std::size_t> position;
		for (std::size_t i{0}; i < size; ++i) {
			position.emplace(inorder[i], i);
		}

		return build(preorder, position, next, 0, size);
		//<
		// int preorder_idx{0};
		// return build(
		//	preorder,
		//	inorder,
		//	pool,
		//	preorder_idx,
		//	inorder.begin(),
		//	inorder.end());
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

	// the trees are equal when they have the same shape and the same values
	bool same_tree(const TreeNode* a, const TreeNode* b) {
		std::vector<std::pair<const TreeNode*, const TreeNode*>> stack{{a, b}};
		while (!stack.empty()) {
			const auto [x, y]{stack.back()};
			stack.pop_back();
			if (x == nullptr || y == nullptr) {
				if (x != y) return false;
				continue;
			}
			if (x->val != y->val) return false;
			stack.push_back({x->left, y->left});
			stack.push_back({x->right, y->right});
		}
		return true;
	}

	void test_build(const std::string& label,
					const std::vector<int>& preorder,
					const std::vector<int>& inorder,
					const std::vector<int>& expected) {
		std::vector<TreeNode> expected_pool;
		const TreeNode* expected_root{build_tree(expected, expected_pool)};

		std::vector<TreeNode> pool;
		pool.reserve(preorder.size());
		const TreeNode* root{build_from_traversals(preorder, inorder, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			same_tree(root, expected_root) ? "OK" : "FAIL") << std::flush;
	}

	// both traversals of a tree, computed without recursion
	void make_traversals(const TreeNode* root, std::vector<int>& preorder, std::vector<int>& inorder) {
		std::vector<const TreeNode*> stack;
		const TreeNode* node{root};
		while (node != nullptr || !stack.empty()) {
			while (node != nullptr) {
				preorder.push_back(node->val);
				stack.push_back(node);
				node = node->left;
			}
			node = stack.back();
			stack.pop_back();
			inorder.push_back(node->val);
			node = node->right;
		}
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// a tree of the given shape, the node 0 is the root; the values are distinct
	std::vector<TreeNode> make_tree(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		std::vector<TreeNode> pool(count);
		for (std::size_t i{0}; i < count; ++i) {
			pool[i].val = static_cast<int>((i * 7919) % 100003) - 50000;
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
			(is_left ? pool[parent].left : pool[parent].right) = &pool[i];
		}
		return pool;
	}

	void test_big(const std::string& label, Shape shape, int n, unsigned seed) {
		const std::vector<TreeNode> original{make_tree(shape, n, seed)};
		std::vector<int> preorder;
		std::vector<int> inorder;
		make_traversals(&original.front(), preorder, inorder);

		std::vector<TreeNode> pool;
		pool.reserve(preorder.size());
		const auto start{std::chrono::steady_clock::now()};
		const TreeNode* root{build_from_traversals(preorder, inorder, pool)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			same_tree(root, &original.front()) ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}
}

int main() {

	test_build("description example 1",
		{3, 9, 20, 15, 7},
		{9, 3, 15, 20, 7},
		{3, 9, 20, nil, nil, 15, 7});
	test_build("description example 2, single node",
		{-1},
		{-1},
		{-1});
	test_build("empty traversals",
		{},
		{},
		{});
	test_build("two nodes, the left child",
		{1, 2},
		{2, 1},
		{1, 2});
	test_build("two nodes, the right child",
		{1, 2},
		{1, 2},
		{1, nil, 2});
	test_build("root with two leaves",
		{2, 1, 3},
		{1, 2, 3},
		{2, 1, 3});
	test_build("left chain of 5, the preorder is the reversed inorder",
		{5, 4, 3, 2, 1},
		{1, 2, 3, 4, 5},
		{5, 4, nil, 3, nil, 2, nil, 1});
	test_build("right chain of 5, the preorder equals the inorder",
		{1, 2, 3, 4, 5},
		{1, 2, 3, 4, 5},
		{1, nil, 2, nil, 3, nil, 4, nil, 5});
	test_build("zigzag of 5",
		{1, 2, 3, 4, 5},
		{2, 4, 5, 3, 1},
		{1, 2, nil, nil, 3, 4, nil, nil, 5});
	test_build("zigzag of 5, starts to the right",
		{1, 2, 3, 4, 5},
		{1, 3, 4, 5, 2},
		{1, nil, 2, 3, nil, nil, 4, nil, 5});
	test_build("complete tree of 7 nodes",
		{4, 2, 1, 3, 6, 5, 7},
		{1, 2, 3, 4, 5, 6, 7},
		{4, 2, 6, 1, 3, 5, 7});
	test_build("complete tree of 15 nodes",
		{8, 4, 2, 1, 3, 6, 5, 7, 12, 10, 9, 11, 14, 13, 15},
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		{8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15});
	test_build("values are not sorted",
		{9, 4, 8, 1, 7, 2, 6},
		{8, 4, 1, 9, 2, 7, 6},
		{9, 4, 7, 8, 1, 2, 6});
	test_build("negative values",
		{-5, -8, -9, -6, -2, -3, -1},
		{-9, -8, -6, -5, -3, -2, -1},
		{-5, -8, -2, -9, -6, -3, -1});
	test_build("only the left child has children",
		{1, 2, 4, 6, 7, 5, 3},
		{6, 4, 7, 2, 5, 1, 3},
		{1, 2, 3, 4, 5, nil, nil, 6, 7});
	test_build("only the right child has children",
		{1, 2, 3, 4, 5, 6, 7},
		{2, 1, 4, 3, 6, 5, 7},
		{1, 2, 3, nil, nil, 4, 5, nil, nil, 6, 7});
	test_build("the left subtree is much deeper",
		{1, 2, 4, 5, 6, 3},
		{6, 5, 4, 2, 1, 3},
		{1, 2, 3, 4, nil, nil, nil, 5, nil, 6});
	test_build("the right subtree is much deeper",
		{1, 2, 3, 4, 5, 6},
		{2, 1, 3, 4, 5, 6},
		{1, 2, 3, nil, nil, nil, 4, nil, 5, nil, 6});
	test_build("a node with a single right child inside the left subtree",
		{1, 2, 4, 3, 5, 6},
		{2, 4, 1, 6, 5, 3},
		{1, 2, 3, nil, 4, 5, nil, nil, nil, 6});
	test_build("a node with a single left child inside the right subtree",
		{1, 2, 4, 3, 5, 6},
		{4, 2, 1, 3, 6, 5},
		{1, 2, 3, 4, nil, nil, 5, nil, nil, 6});
	test_build("zero among the values",
		{0, -1, -2, 1, 2},
		{-2, -1, 0, 1, 2},
		{0, -1, 1, -2, nil, nil, 2});
	test_build("random 1, n = 3",
		{51, -11, 48},
		{-11, 51, 48},
		{51, -11, 48});
	test_build("random 2, n = 5",
		{-21, 96, 46, 34, -15},
		{46, 96, -21, 34, -15},
		{-21, 96, 34, 46, nil, nil, -15});
	test_build("random 3, n = 8",
		{92, -63, -75, -57, 68, 1, -11, 10},
		{92, -57, -75, 68, -63, 1, 10, -11},
		{92, nil, -63, -75, 1, -57, 68, nil, -11, nil, nil, nil, nil, 10});
	test_build("random 4, n = 12",
		{34, -36, 72, -53, -85, -17, -9, 59, -71, 85, 88, 95},
		{34, -53, 72, -36, -85, -9, -17, 88, 85, 95, -71, 59},
		{34, nil, -36, 72, -85, -53, nil, nil, -17, nil, nil, -9, 59, nil, nil, -71, nil, 85, nil, 88, 95});
	test_build("random 5, n = 20",
		{-9, 23, -26, 80, 41, 40, 96, -53, -33, 44, -68, 48, -52, 56, -72, 100, 0, -48, -97, -86},
		{23, -9, -26, 40, 41, 96, -53, 80, 56, -52, 48, -68, 44, -72, -33, 100, -48, 0, -97, -86},
		{-9, 23, -26, nil, nil, nil, 80, 41, -33, 40, 96, 44, 100, nil, nil, nil, -53, -68, -72, nil, 0, nil, nil, 48, nil, nil, nil, -48, -97, -52, nil, nil, nil, nil, -86, 56});
	test_build("random 6, n = 30",
		{-79, -72, 25, -73, -58, 11, 59, -38, 79, -29, 35, -89, -27, -45, 86, -7, -46, 75, 36, -53, 20, -13, -78, -70, 58, -31, -98, -40, 24, 32},
		{25, -72, -38, 79, 59, -29, 11, 35, -58, -27, 86, -45, -46, -7, 75, 36, -89, -73, -53, 20, -79, -31, 58, -70, -78, -98, -13, 24, -40, 32},
		{-79, -72, -13, 25, -73, -78, -40, nil, nil, -58, -53, -70, -98, 24, 32, 11, -89, nil, 20, 58, nil, nil, nil, nil, nil, nil, nil, 59, 35, -27, nil, nil, nil, -31, nil, -38, -29, nil, nil, nil, -45, nil, nil, nil, 79, nil, nil, 86, -7, nil, nil, nil, nil, -46, 75, nil, nil, nil, 36});
	test_build("random 7, n = 40",
		{99, -92, -66, -93, 43, -78, -56, -55, -94, -37, -8, 53, 90, -23, 67, 45, 21, -53, -77, -20, 13, 86, 69, -11, 89, 74, 73, -4, -46, 22, -100, 61, -82, -91, 15, -47, 95, 75, 20, 6},
		{-93, -66, -92, -55, -56, -37, 90, 53, -8, -94, -23, -78, 45, 67, -53, 21, 43, -20, -77, 86, 13, 99, -11, 89, 74, -4, 73, 69, 22, -82, 61, -100, -46, 95, -47, 15, -91, 75, 20, 6},
		{99, -92, 69, -66, 43, -11, -46, -93, nil, -78, -77, nil, 89, 22, -91, nil, nil, -56, 67, -20, 13, nil, 74, nil, -100, 15, 75, -55, -94, 45, 21, nil, nil, 86, nil, nil, 73, 61, nil, -47, nil, nil, 20, nil, nil, -37, -23, nil, nil, -53, nil, nil, nil, -4, nil, -82, nil, 95, nil, nil, 6, nil, -8, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 53, nil, 90});
	test_build("random 8, n = 50",
		{-33, 79, -14, 4, 82, 97, -17, 98, -35, -84, -9, 89, -53, 20, -47, -4, -25, 1, -37, 60, 90, 9, -67, 7, -23, 22, 78, 92, 80, -81, 51, 88, -90, 14, 17, 50, 8, 21, 96, 11, 71, -3, -30, -15, -94, -18, -97, 72, 34, 56},
		{4, 97, -17, 82, -14, -35, -84, 98, 79, -53, 89, -9, 20, -33, 90, 9, 60, -67, -37, 1, 22, 78, -23, 80, 92, 7, -81, 51, 14, -90, 88, -25, 17, -4, 96, 21, 71, 11, 8, 50, -3, -30, -47, -18, -94, -97, 72, -15, 34, 56},
		{-33, 79, -47, -14, -9, -4, -15, 4, 98, 89, 20, -25, 50, -94, 34, nil, 82, -35, nil, -53, nil, nil, nil, 1, 17, 8, -3, -18, -97, nil, 56, 97, nil, nil, -84, nil, nil, -37, 7, nil, nil, 21, nil, nil, -30, nil, nil, nil, 72, nil, nil, nil, -17, nil, nil, 60, nil, -23, -81, 96, 11, nil, nil, nil, nil, nil, nil, 90, -67, 22, 92, nil, 51, nil, nil, 71, nil, nil, 9, nil, nil, nil, 78, 80, nil, nil, 88, nil, nil, nil, nil, nil, nil, nil, nil, -90, nil, 14});

	test_big("n = 100000, complete tree", Shape::complete, 100000, 0);
	test_big("n = 100000, left chain", Shape::left_chain, 100000, 0);
	test_big("n = 100000, right chain", Shape::right_chain, 100000, 0);
	test_big("n = 100000, zigzag", Shape::zigzag, 100000, 0);
	test_big("n = 100000, random tree", Shape::random, 100000, 1);
	test_big("n = 100000, another random tree", Shape::random, 100000, 2);

	return 0;
}
```