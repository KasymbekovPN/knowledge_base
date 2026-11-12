Задача 4 блока 7: Binary Tree Level Order Traversal. Беру обвязку из прошлой задачи, собираю тесты и проверяю их запуском.

Проверяю эталоны под санитайзерами, затем отправлю задачу.

Задача 4 блока 7 готова. Тесты проверены запуском: пустая заглушка даёт 36 FAIL из 37, два разных эталона проходят все 37 без предупреждений и чисто под ASan/UBSan, три неверных варианта (змейкой, правый потомок раньше левого, то же в рекурсии) ловятся.

## Binary Tree Level Order Traversal

Дано бинарное дерево. Верните значения его узлов по уровням: сверху вниз, а внутри уровня слева направо. Результат это `std::vector<std::vector<int>>`, где `i`-й вектор содержит значения узлов глубины `i`. Для пустого дерева результат пустой.

```cpp
std::vector<std::vector<int>> level_order(const TreeNode* root);
```

**Ограничения:** 0 ≤ n ≤ 10⁵, значения от −100 до 100.

**Вопросы интервьюера:**

1. Какая сложность по времени и по памяти? Чем память определяется (кроме самого результата)?
2. Как вы отделяете один уровень от другого? Можно ли обойтись одной очередью без маркеров и без хранения `nullptr`?
3. Можно ли решить рекурсивно? Что поменяется в оценке памяти и на цепочке из 10⁵ узлов?
4. Как переделать обход так, чтобы чётные уровни шли слева направо, а нечётные справа налево?

**Цель:** O(n) по времени. **Ориентир по времени:** 12 минут, лучше с первого запуска без предупреждений.

```cpp
namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	std::vector<std::vector<int>> level_order([[maybe_unused]] const TreeNode* root) {
		return {};
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

	// reference answer for large trees: depth-first with an explicit stack,
	// the left child is visited before the right one, so every level is filled left to right
	std::vector<std::vector<int>> expected_levels(const TreeNode* root) {
		std::vector<std::vector<int>> result;
		std::vector<std::pair<const TreeNode*, std::size_t>> stack;
		if (root != nullptr) {
			stack.push_back({root, 0});
		}
		while (!stack.empty()) {
			const auto [node, depth]{stack.back()};
			stack.pop_back();
			if (depth == result.size()) {
				result.emplace_back();
			}
			result[depth].push_back(node->val);
			if (node->right != nullptr) stack.push_back({node->right, depth + 1});
			if (node->left != nullptr) stack.push_back({node->left, depth + 1});
		}
		return result;
	}

	void test_levels(const std::string& label,
					 const std::vector<int>& level,
					 const std::vector<std::vector<int>>& expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			level_order(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	void test_big(const std::string& label, const TreeNode* root) {
		const std::vector<std::vector<int>> expected{expected_levels(root)};

		const auto start{std::chrono::steady_clock::now()};
		const std::vector<std::vector<int>> result{level_order(root)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// the nodes 0..n-1 of a tree of the given shape, node 0 is the root
	std::vector<TreeNode> make_tree(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		std::vector<TreeNode> pool(count);
		for (std::size_t i{0}; i < count; ++i) {
			pool[i].val = static_cast<int>((i * 7) % 201) - 100;
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
}

int main() {

	test_levels("description example 1",
		{3, 9, 20, nil, nil, 15, 7},
		{{3}, {9, 20}, {15, 7}});
	test_levels("description example 2, single node",
		{1},
		{{1}});
	test_levels("empty tree",
		{},
		{});
	test_levels("root with two leaves",
		{1, 2, 3},
		{{1}, {2, 3}});
	test_levels("root with a left child only",
		{1, 2},
		{{1}, {2}});
	test_levels("root with a right child only",
		{1, nil, 2},
		{{1}, {2}});
	test_levels("left chain of 5",
		{1, 2, nil, 3, nil, 4, nil, 5},
		{{1}, {2}, {3}, {4}, {5}});
	test_levels("right chain of 5",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		{{1}, {2}, {3}, {4}, {5}});
	test_levels("zigzag of 5",
		{1, 2, nil, nil, 3, 4, nil, nil, 5},
		{{1}, {2}, {3}, {4}, {5}});
	test_levels("complete tree of 7 nodes",
		{1, 2, 3, 4, 5, 6, 7},
		{{1}, {2, 3}, {4, 5, 6, 7}});
	test_levels("complete tree of 15 nodes",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		{{1}, {2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11, 12, 13, 14, 15}});
	test_levels("complete tree of 15 nodes, negative values",
		{-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15},
		{{-1}, {-2, -3}, {-4, -5, -6, -7}, {-8, -9, -10, -11, -12, -13, -14, -15}});
	test_levels("last level is only partially filled",
		{1, 2, 3, 4, 5, 6, 7, 8, 9},
		{{1}, {2, 3}, {4, 5, 6, 7}, {8, 9}});
	test_levels("only the left subtree is deep",
		{1, 2, 3, 4, nil, nil, nil, 5, nil, 6},
		{{1}, {2, 3}, {4}, {5}, {6}});
	test_levels("only the right subtree is deep",
		{1, 2, 3, nil, nil, nil, 4, nil, 5},
		{{1}, {2, 3}, {4}, {5}});
	test_levels("holes in the middle of a level",
		{1, 2, 3, nil, 4, 5, nil, nil, nil, 6, 7},
		{{1}, {2, 3}, {4, 5}, {6, 7}});
	test_levels("gaps between nodes of one level",
		{1, 2, 3, 4, nil, nil, 5, 6, nil, nil, nil, nil, 7},
		{{1}, {2, 3}, {4, 5}, {6}, {7}});
	test_levels("equal values everywhere",
		{7, 7, 7, 7, 7, 7, 7},
		{{7}, {7, 7}, {7, 7, 7, 7}});
	test_levels("zero values",
		{0, 0, 0, 0, 0, 0, 0},
		{{0}, {0, 0}, {0, 0, 0, 0}});
	test_levels("extreme values",
		{100, -100, 100, -100, 100, -100, 100},
		{{100}, {-100, 100}, {-100, 100, -100, 100}});
	test_levels("left child has a right child, right child has a left child",
		{1, 2, 3, nil, 4, 5},
		{{1}, {2, 3}, {4, 5}});
	test_levels("the second level has a single node",
		{1, 2, nil, 3, 4},
		{{1}, {2}, {3, 4}});
	test_levels("right child with a left child",
		{1, nil, 2, 3},
		{{1}, {2}, {3}});
	test_levels("random 1, n = 2",
		{-63, nil, -57},
		{{-63}, {-57}});
	test_levels("random 2, n = 3",
		{41, -2, nil, 58},
		{{41}, {-2}, {58}});
	test_levels("random 3, n = 5",
		{9, 85, nil, 88, 39, 13},
		{{9}, {85}, {88, 39}, {13}});
	test_levels("random 4, n = 8",
		{-42, -79, 76, nil, 55, nil, 25, nil, 13, nil, nil, -44, nil, 35},
		{{-42}, {-79, 76}, {55, 25}, {13}, {-44}, {35}});
	test_levels("random 5, n = 12",
		{59, -37, 61, -58, -77, 71, nil, nil, nil, nil, nil, 50, -4, nil, nil, -51, nil, 91, 1, -47},
		{{59}, {-37, 61}, {-58, -77, 71}, {50, -4}, {-51}, {91, 1}, {-47}});
	test_levels("random 6, n = 20",
		{-40, 42, 10, -20, 71, nil, 1, 0, 90, nil, 63, nil, nil, -22, -6, 90, 53, 99, 60, -2, nil, nil, nil, 38, -59, nil, nil, nil, nil, 21, 74},
		{{-40}, {42, 10}, {-20, 71, 1}, {0, 90, 63}, {-22, -6, 90, 53, 99, 60}, {-2, 38, -59, 21, 74}});
	test_levels("random 7, n = 30",
		{85, 19, 6, nil, nil, -20, -56, -52, 53, 32, -39, 98, -60, -57, -30, -65, nil, 10, nil, -30, -34, -98, -71, nil, nil, 100, -32, nil, 36, nil, nil, nil, nil, nil, nil, nil, 75, nil, 90, nil, 18, nil, nil, -99, nil, nil, -47, -49, nil, nil, nil, nil, 6, nil, nil, nil, 89},
		{{85}, {19, 6}, {-20, -56}, {-52, 53, 32, -39}, {98, -60, -57, -30, -65, 10}, {-30, -34, -98, -71, 100, -32, 36}, {75, 90, 18, -99}, {-47, -49, 6}, {89}});
	test_levels("random 8, n = 50",
		{82, nil, 71, -99, 20, -35, -44, 66, -30, nil, nil, -30, 86, 79, 94, -82, 80, -77, nil, 39, 49, nil, -66, nil, nil, nil, -69, -72, -85, nil, 92, -34, nil, 63, nil, 77, -69, 71, 19, 23, 99, 42, 81, nil, nil, nil, nil, nil, nil, nil, -30, 68, nil, nil, 89, nil, -98, nil, -50, -65, 79, nil, nil, 74, 3, nil, nil, nil, nil, 20, 13, nil, nil, nil, nil, -61, nil, nil, nil, 1, -31, nil, nil, nil, nil, nil, 23, nil, nil, nil, nil, -80, nil, nil, 48, nil, nil, -66},
		{{82}, {71}, {-99, 20}, {-35, -44, 66, -30}, {-30, 86, 79, 94, -82, 80}, {-77, 39, 49, -66, -69, -72, -85}, {92, -34, 63, 77, -69, 71, 19, 23, 99, 42, 81}, {-30, 68, 89, -98, -50, -65, 79, 74, 3}, {20, 13, -61, 1, -31}, {23, -80}, {48}, {-66}});

	{
		std::vector<TreeNode> pool{make_tree(Shape::complete, 100000, 0)};
		test_big("n = 100000, complete tree", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_tree(Shape::left_chain, 100000, 0)};
		test_big("n = 100000, left chain", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_tree(Shape::right_chain, 100000, 0)};
		test_big("n = 100000, right chain", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_tree(Shape::zigzag, 100000, 0)};
		test_big("n = 100000, zigzag", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_tree(Shape::random, 100000, 1)};
		test_big("n = 100000, random tree", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_tree(Shape::random, 100000, 2)};
		test_big("n = 100000, another random tree", &pool.front());
	}

	return 0;
}
```

Проверка по времени (1 секунда) не умеет прерывать зависшую программу: если решение зациклится, остановите его сами. Перед отправкой скомпилируйте и прогоните.

Пришлите решение вложением. Когда будет готово, я его разберу и закончу списком «в журнал ошибок».

---

15:30 -  15:44

---

---


```cpp
#include <iostream>
#include <format>
#include <utility>
#include <tuple>
#include <queue>
#include <vector>
#include <string>
#include <random>
#include <chrono>

namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	std::vector<std::vector<int>> level_order(const TreeNode* root) {
		std::vector<std::vector<int>> answer;
		if (root == nullptr) return answer;
		
		std::queue<const TreeNode*> buffer;
		buffer.push(root);
		answer.push_back({root->val});
		
		while (!buffer.empty()) {
			std::vector<int> item{};
			const std::size_t size{buffer.size()};
			for (std::size_t i{0}; i < size; ++i) {
				const TreeNode* node{buffer.front()};
				buffer.pop();
				
				if (node->left != nullptr) {
					buffer.push(node->left);
					item.push_back(node->left->val);
				}
				if (node->right != nullptr) {
					buffer.push(node->right);
					item.push_back(node->right->val);
				}
			}
			
			if (!item.empty()) {
				answer.push_back(std::move(item));
			}
		}
	
		return answer;
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

	// reference answer for large trees: depth-first with an explicit stack,
	// the left child is visited before the right one, so every level is filled left to right
	std::vector<std::vector<int>> expected_levels(const TreeNode* root) {
		std::vector<std::vector<int>> result;
		std::vector<std::pair<const TreeNode*, std::size_t>> stack;
		if (root != nullptr) {
			stack.push_back({root, 0});
		}
		while (!stack.empty()) {
			const auto [node, depth]{stack.back()};
			stack.pop_back();
			if (depth == result.size()) {
				result.emplace_back();
			}
			result[depth].push_back(node->val);
			if (node->right != nullptr) stack.push_back({node->right, depth + 1});
			if (node->left != nullptr) stack.push_back({node->left, depth + 1});
		}
		return result;
	}

	void test_levels(const std::string& label,
					 const std::vector<int>& level,
					 const std::vector<std::vector<int>>& expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			level_order(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	void test_big(const std::string& label, const TreeNode* root) {
		const std::vector<std::vector<int>> expected{expected_levels(root)};

		const auto start{std::chrono::steady_clock::now()};
		const std::vector<std::vector<int>> result{level_order(root)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// the nodes 0..n-1 of a tree of the given shape, node 0 is the root
	std::vector<TreeNode> make_tree(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		std::vector<TreeNode> pool(count);
		for (std::size_t i{0}; i < count; ++i) {
			pool[i].val = static_cast<int>((i * 7) % 201) - 100;
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
}

int main() {

	test_levels("description example 1",
		{3, 9, 20, nil, nil, 15, 7},
		{{3}, {9, 20}, {15, 7}});
	test_levels("description example 2, single node",
		{1},
		{{1}});
	test_levels("empty tree",
		{},
		{});
	test_levels("root with two leaves",
		{1, 2, 3},
		{{1}, {2, 3}});
	test_levels("root with a left child only",
		{1, 2},
		{{1}, {2}});
	test_levels("root with a right child only",
		{1, nil, 2},
		{{1}, {2}});
	test_levels("left chain of 5",
		{1, 2, nil, 3, nil, 4, nil, 5},
		{{1}, {2}, {3}, {4}, {5}});
	test_levels("right chain of 5",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		{{1}, {2}, {3}, {4}, {5}});
	test_levels("zigzag of 5",
		{1, 2, nil, nil, 3, 4, nil, nil, 5},
		{{1}, {2}, {3}, {4}, {5}});
	test_levels("complete tree of 7 nodes",
		{1, 2, 3, 4, 5, 6, 7},
		{{1}, {2, 3}, {4, 5, 6, 7}});
	test_levels("complete tree of 15 nodes",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		{{1}, {2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11, 12, 13, 14, 15}});
	test_levels("complete tree of 15 nodes, negative values",
		{-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15},
		{{-1}, {-2, -3}, {-4, -5, -6, -7}, {-8, -9, -10, -11, -12, -13, -14, -15}});
	test_levels("last level is only partially filled",
		{1, 2, 3, 4, 5, 6, 7, 8, 9},
		{{1}, {2, 3}, {4, 5, 6, 7}, {8, 9}});
	test_levels("only the left subtree is deep",
		{1, 2, 3, 4, nil, nil, nil, 5, nil, 6},
		{{1}, {2, 3}, {4}, {5}, {6}});
	test_levels("only the right subtree is deep",
		{1, 2, 3, nil, nil, nil, 4, nil, 5},
		{{1}, {2, 3}, {4}, {5}});
	test_levels("holes in the middle of a level",
		{1, 2, 3, nil, 4, 5, nil, nil, nil, 6, 7},
		{{1}, {2, 3}, {4, 5}, {6, 7}});
	test_levels("gaps between nodes of one level",
		{1, 2, 3, 4, nil, nil, 5, 6, nil, nil, nil, nil, 7},
		{{1}, {2, 3}, {4, 5}, {6}, {7}});
	test_levels("equal values everywhere",
		{7, 7, 7, 7, 7, 7, 7},
		{{7}, {7, 7}, {7, 7, 7, 7}});
	test_levels("zero values",
		{0, 0, 0, 0, 0, 0, 0},
		{{0}, {0, 0}, {0, 0, 0, 0}});
	test_levels("extreme values",
		{100, -100, 100, -100, 100, -100, 100},
		{{100}, {-100, 100}, {-100, 100, -100, 100}});
	test_levels("left child has a right child, right child has a left child",
		{1, 2, 3, nil, 4, 5},
		{{1}, {2, 3}, {4, 5}});
	test_levels("the second level has a single node",
		{1, 2, nil, 3, 4},
		{{1}, {2}, {3, 4}});
	test_levels("right child with a left child",
		{1, nil, 2, 3},
		{{1}, {2}, {3}});
	test_levels("random 1, n = 2",
		{-63, nil, -57},
		{{-63}, {-57}});
	test_levels("random 2, n = 3",
		{41, -2, nil, 58},
		{{41}, {-2}, {58}});
	test_levels("random 3, n = 5",
		{9, 85, nil, 88, 39, 13},
		{{9}, {85}, {88, 39}, {13}});
	test_levels("random 4, n = 8",
		{-42, -79, 76, nil, 55, nil, 25, nil, 13, nil, nil, -44, nil, 35},
		{{-42}, {-79, 76}, {55, 25}, {13}, {-44}, {35}});
	test_levels("random 5, n = 12",
		{59, -37, 61, -58, -77, 71, nil, nil, nil, nil, nil, 50, -4, nil, nil, -51, nil, 91, 1, -47},
		{{59}, {-37, 61}, {-58, -77, 71}, {50, -4}, {-51}, {91, 1}, {-47}});
	test_levels("random 6, n = 20",
		{-40, 42, 10, -20, 71, nil, 1, 0, 90, nil, 63, nil, nil, -22, -6, 90, 53, 99, 60, -2, nil, nil, nil, 38, -59, nil, nil, nil, nil, 21, 74},
		{{-40}, {42, 10}, {-20, 71, 1}, {0, 90, 63}, {-22, -6, 90, 53, 99, 60}, {-2, 38, -59, 21, 74}});
	test_levels("random 7, n = 30",
		{85, 19, 6, nil, nil, -20, -56, -52, 53, 32, -39, 98, -60, -57, -30, -65, nil, 10, nil, -30, -34, -98, -71, nil, nil, 100, -32, nil, 36, nil, nil, nil, nil, nil, nil, nil, 75, nil, 90, nil, 18, nil, nil, -99, nil, nil, -47, -49, nil, nil, nil, nil, 6, nil, nil, nil, 89},
		{{85}, {19, 6}, {-20, -56}, {-52, 53, 32, -39}, {98, -60, -57, -30, -65, 10}, {-30, -34, -98, -71, 100, -32, 36}, {75, 90, 18, -99}, {-47, -49, 6}, {89}});
	test_levels("random 8, n = 50",
		{82, nil, 71, -99, 20, -35, -44, 66, -30, nil, nil, -30, 86, 79, 94, -82, 80, -77, nil, 39, 49, nil, -66, nil, nil, nil, -69, -72, -85, nil, 92, -34, nil, 63, nil, 77, -69, 71, 19, 23, 99, 42, 81, nil, nil, nil, nil, nil, nil, nil, -30, 68, nil, nil, 89, nil, -98, nil, -50, -65, 79, nil, nil, 74, 3, nil, nil, nil, nil, 20, 13, nil, nil, nil, nil, -61, nil, nil, nil, 1, -31, nil, nil, nil, nil, nil, 23, nil, nil, nil, nil, -80, nil, nil, 48, nil, nil, -66},
		{{82}, {71}, {-99, 20}, {-35, -44, 66, -30}, {-30, 86, 79, 94, -82, 80}, {-77, 39, 49, -66, -69, -72, -85}, {92, -34, 63, 77, -69, 71, 19, 23, 99, 42, 81}, {-30, 68, 89, -98, -50, -65, 79, 74, 3}, {20, 13, -61, 1, -31}, {23, -80}, {48}, {-66}});

	{
		std::vector<TreeNode> pool{make_tree(Shape::complete, 100000, 0)};
		test_big("n = 100000, complete tree", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_tree(Shape::left_chain, 100000, 0)};
		test_big("n = 100000, left chain", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_tree(Shape::right_chain, 100000, 0)};
		test_big("n = 100000, right chain", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_tree(Shape::zigzag, 100000, 0)};
		test_big("n = 100000, zigzag", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_tree(Shape::random, 100000, 1)};
		test_big("n = 100000, random tree", &pool.front());
	}
	{
		std::vector<TreeNode> pool{make_tree(Shape::random, 100000, 2)};
		test_big("n = 100000, another random tree", &pool.front());
	}

	return 0;
}
```