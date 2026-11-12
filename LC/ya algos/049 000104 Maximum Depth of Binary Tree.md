
Начинаю блок 7 (деревья), первая задача — Maximum Depth of Binary Tree. Собираю шаблон с построением дерева из уровневого списка и проверяю его прогонами; скоро пришлю.

Блок 7 (деревья), задача 1: **Maximum Depth of Binary Tree** (LC 104). Это разминка перед остальными задачами на деревья, которые построены на обходе в глубину и в ширину.

**Условие.** Дан корень бинарного дерева `root` (может быть `nullptr`). Верните его максимальную глубину: количество узлов на самом длинном пути от корня до листа. У пустого дерева глубина `0`, у дерева из одного узла `1`.

**Сигнатура** (уже стоит в шаблоне как заглушка):

```cpp
int max_depth(const TreeNode* root);
```

Узел определён в шаблоне:

```cpp
struct TreeNode {
    int val{0};
    TreeNode* left{nullptr};
    TreeNode* right{nullptr};
};
```

**Ограничения:** `0 <= число узлов <= 100000`, `-100 <= val <= 100`. Дерево может быть любой формы, в том числе вырожденной цепочкой.

**Вопросы интервьюера:**

1. Какая сложность по времени и по памяти? От чего зависит память?
2. Что произойдёт на вырожденном дереве из 100 000 узлов в рекурсивном решении? Как сделать то же без рекурсии?
3. Чем отличается обход в глубину от обхода в ширину для этой задачи? Какой из них вы выберете и почему?
4. Как изменится решение, если нужна минимальная глубина (до ближайшего листа)? Где подвох?

**Цель:** O(n) по времени, память O(h) для рекурсии или O(w) для обхода в ширину (h высота дерева, w максимальная ширина). Любое из двух решений годится, но интервьюер попросит обсудить оба.

**Время:** 15 минут. Критерий «выучено»: ≤25 минут, компиляция с первого раза, ни одного предупреждения.

Деревья в тестах строятся из уровневого списка (как в LeetCode): `nil` обозначает отсутствующего потомка. Узлы лежат в `std::vector<TreeNode>`, поэтому тестовый код сам следит за их временем жизни, вам нужно только читать дерево.

```cpp
namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	int max_depth([[maybe_unused]] const TreeNode* root) {
		return 0;
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

	// reference answer: breadth-first traversal, no recursion
	int expected_depth(const TreeNode* root) {
		if (root == nullptr) {
			return 0;
		}
		int depth{0};
		std::vector<const TreeNode*> current{root};
		while (!current.empty()) {
			++depth;
			std::vector<const TreeNode*> next;
			for (const TreeNode* node : current) {
				if (node->left != nullptr) next.push_back(node->left);
				if (node->right != nullptr) next.push_back(node->right);
			}
			current = std::move(next);
		}
		return depth;
	}

	void test_depth(const std::string& label,
					const std::vector<int>& level,
					int expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			max_depth(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	void test_big(const std::string& label, const TreeNode* root) {
		const int expected{expected_depth(root)};

		const auto start{std::chrono::steady_clock::now()};
		const int result{max_depth(root)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	// a chain: every node has one child, on the left or on the right
	TreeNode* make_chain(std::vector<TreeNode>& pool, int n, bool to_left) {
		pool.assign(static_cast<std::size_t>(n), TreeNode{});
		for (std::size_t i{0}; i < pool.size(); ++i) {
			pool[i].val = static_cast<int>(i % 100);
			if (i + 1 < pool.size()) {
				(to_left ? pool[i].left : pool[i].right) = &pool[i + 1];
			}
		}
		return &pool.front();
	}

	// a chain that turns left, right, left, right, ...
	TreeNode* make_zigzag(std::vector<TreeNode>& pool, int n) {
		pool.assign(static_cast<std::size_t>(n), TreeNode{});
		for (std::size_t i{0}; i < pool.size(); ++i) {
			if (i + 1 < pool.size()) {
				(i % 2 == 0 ? pool[i].left : pool[i].right) = &pool[i + 1];
			}
		}
		return &pool.front();
	}

	// the node i has the children 2i + 1 and 2i + 2
	TreeNode* make_complete(std::vector<TreeNode>& pool, int n) {
		pool.assign(static_cast<std::size_t>(n), TreeNode{});
		for (std::size_t i{0}; i < pool.size(); ++i) {
			if (2 * i + 1 < pool.size()) pool[i].left = &pool[2 * i + 1];
			if (2 * i + 2 < pool.size()) pool[i].right = &pool[2 * i + 2];
		}
		return &pool.front();
	}

	// every new node is attached to a random free place
	TreeNode* make_random(std::vector<TreeNode>& pool, int n, unsigned seed) {
		pool.assign(static_cast<std::size_t>(n), TreeNode{});
		std::mt19937 gen{seed};
		std::vector<std::pair<std::size_t, bool>> free_places{{0, true}, {0, false}};
		for (std::size_t i{1}; i < pool.size(); ++i) {
			const std::size_t pick{gen() % free_places.size()};
			const auto [parent, is_left] = free_places[pick];
			free_places[pick] = free_places.back();
			free_places.pop_back();
			(is_left ? pool[parent].left : pool[parent].right) = &pool[i];
			free_places.push_back({i, true});
			free_places.push_back({i, false});
		}
		return &pool.front();
	}
}

int main() {
	test_depth("description example 1",
		{3, 9, 20, nil, nil, 15, 7},
		3);
	test_depth("description example 2",
		{1, nil, 2},
		2);
	test_depth("empty tree",
		{},
		0);
	test_depth("single node",
		{5},
		1);
	test_depth("root with left child only",
		{1, 2},
		2);
	test_depth("root with right child only",
		{1, nil, 2},
		2);
	test_depth("root with two leaves",
		{1, 2, 3},
		2);
	test_depth("left chain of 4",
		{1, 2, nil, 3, nil, 4},
		4);
	test_depth("right chain of 4",
		{1, nil, 2, nil, 3, nil, 4},
		4);
	test_depth("zigzag chain",
		{1, 2, nil, nil, 3, 4},
		4);
	test_depth("deep side is the right one",
		{1, 2, 3, nil, nil, nil, 4, nil, 5},
		4);
	test_depth("deep side is the left one",
		{1, 2, 3, 4, nil, nil, nil, 5},
		4);
	test_depth("complete tree of 7 nodes",
		{1, 2, 3, 4, 5, 6, 7},
		3);
	test_depth("complete tree of 15 nodes",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		4);
	test_depth("both subtrees deep, left is deeper",
		{1, 2, 3, 4, nil, 5, nil, 6, nil},
		4);
	test_depth("depth equal at both sides",
		{1, 2, 3, 4, nil, nil, 5},
		3);
	test_depth("deepest leaf in the middle",
		{1, 2, 3, nil, 4, 5, nil, nil, nil, 6},
		4);
	test_depth("negative values",
		{-1, -2, -3, nil, -4},
		3);
	test_depth("zero values",
		{0, 0, 0, nil, 0},
		3);
	test_depth("extreme values",
		{100, -100, 100, nil, -100},
		3);
	test_depth("long left chain with a short right child",
		{1, 2, 3, 4, nil, nil, nil, 5, nil, 6},
		5);
	test_depth("random 1, n=6",
		{-63, -70, -81, nil, 67, nil, 92, -16},
		4);
	test_depth("random 2, n=9",
		{-38, -34, -54, 45, nil, nil, -90, -75, nil, -91, nil, nil, nil, 53, nil, nil, -69},
		6);
	test_depth("random 3, n=12",
		{-92, -63, 18, -59, -46, nil, nil, 59, 13, nil, -79, nil, -85, -61, -82, nil, nil, nil, nil, -98},
		6);
	test_depth("random 4, n=17",
		{-69, -77, -51, 48, -38, 51, -78, 56, nil, 67, -22, -74, nil, 44, nil, nil, nil, nil, -58, nil, nil, 62, -82, nil, nil, nil, nil, nil, nil, 67, -98},
		6);
	test_depth("random 5, n=25",
		{31, nil, -66, 28, -9, -90, 98, 48, -60, 17, -98, -56, 43, -31, 9, nil, nil, nil, nil, nil, -93, nil, nil, -33, nil, -11, 35, 16, nil, nil, nil, nil, nil, -91, 78, nil, -55, -58, nil, nil, nil, nil, nil, -58, 24},
		8);
	test_depth("random 6, n=40",
		{55, 39, -38, -78, 6, nil, -6, 89, -38, 68, 66, -59, nil, 32, -49, -70, nil, nil, 32, 72, -18, nil, 20, -33, 11, nil, 75, nil, nil, -18, nil, 20, 79, -86, 63, 15, nil, nil, nil, 97, nil, -31, nil, 58, nil, 67, 51, nil, nil, 99, -88, nil, nil, nil, nil, 1, nil, nil, nil, nil, nil, nil, nil, nil, -19, 71, 28, nil, nil, 72, 100},
		9);
	test_depth("random 7, n=60",
		{-76, 47, -62, 55, -40, 86, -46, 8, 37, 91, 27, 80, 53, -1, -82, 67, 76, 33, 58, -50, nil, nil, 46, 79, 13, -84, 73, -50, -79, -22, 23, 85, -90, nil, nil, nil, -21, 2, nil, nil, nil, -41, 70, nil, nil, -53, -31, 47, 27, nil, -28, nil, 81, nil, -9, nil, nil, -75, -95, nil, 41, nil, nil, 28, nil, nil, nil, -68, nil, nil, 1, nil, nil, -15, nil, -69, 10, nil, 67, nil, nil, nil, nil, -15, -90, -81, nil, -79, nil, nil, nil, nil, nil, nil, nil, nil, nil, 76, nil, nil, nil, nil, nil, nil, -22, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, -8, nil, -7},
		10);
	test_depth("random 8, n=80",
		{68, 23, 52, -29, 39, -30, -66, 65, 86, -10, -98, 84, -44, 23, 13, 34, nil, 53, -56, 42, -85, 25, nil, -62, 49, -62, nil, 35, -19, -33, -14, nil, nil, 14, 52, nil, nil, nil, -96, nil, -74, nil, nil, -35, nil, -47, -74, -86, 63, 29, nil, 63, 54, -19, 28, -6, -57, nil, nil, nil, nil, nil, nil, nil, nil, -46, nil, 57, nil, nil, 56, nil, 55, nil, nil, 9, 96, nil, -42, 16, 78, nil, nil, nil, nil, 87, 87, nil, -3, nil, nil, 25, 41, nil, nil, nil, 36, 6, -67, nil, 72, 9, 95, 22, nil, nil, -41, nil, nil, nil, 83, nil, nil, nil, nil, nil, nil, nil, nil, -92, 46, nil, nil, 65, 77, nil, -56, nil, -24, nil, -19, nil, nil, nil, nil, nil, 62, nil, nil, nil, nil, 45, nil, nil, nil, 74, nil, nil, nil, -64, 23, nil, 84},
		11);

	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, chain to the left", make_chain(pool, 100000, true));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, chain to the right", make_chain(pool, 100000, false));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, zigzag chain", make_zigzag(pool, 100000));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, complete tree", make_complete(pool, 100000));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, random shape", make_random(pool, 100000, 1));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, random shape, other seed", make_random(pool, 100000, 2));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 1, single node", make_chain(pool, 1, true));
	}

	return 0;
}
```

Проверка шаблона на реальных прогонах: заглушка проходит 1 тест из 36 (пустое дерево), рекурсивный и итеративный эталоны проходят все 36. Рекурсивный проходит и в `-O0`, и в `-O2` на цепочках из 100 000 узлов со стеком 8 МБ, но под `-fsanitize=address` падает с `stack-overflow`: фреймы санитайзера крупнее. Это штатное поведение, не ошибка шаблона, и это ровно вопрос 2 интервьюера. Пять заведомо неверных вариантов шаблон ловит: `min` вместо `max`, подсчёт узлов, обход только левого потомка, возврат `0` на листе (сдвиг на единицу) и спуск только по первому пути. Ожидаемые глубины маленьких тестов посчитаны на Python независимой реализацией, а большие сверяются обходом в ширину без рекурсии.

Пришлите решение, когда будет готово.

---

22:03 - 22:26

---

```
     1                // {1}
    / \
 1       1            // {2, 2}
/ \     / \
1  1    1  1          // {3, 3, 3, 3}
    \  / \
     1 1  1           // {3, 3, 3, 4, 4, 4, 3, 3}
    /
   1                  // {3, 3, 3, 3, 3, 3, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3,}
```

---

```cpp
#include <iostream>
#include <format>
#include <cstddef>
#include <vector>
#include <queue>
#include <string>
#include <chrono>
#include <random>

namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	int max_depth(const TreeNode* root) {
		if (root == nullptr) return 0;
		
		int depth{0};
		std::queue<const TreeNode*> q;
		q.push(root);
		
		while (!q.empty()) {
			const int size{static_cast<int>(q.size())};
			for (int i{0}; i < size; ++i) {
				auto node{q.front()};
				q.pop();
				
				if (node->left != nullptr) q.push(node->left);
				if (node->right != nullptr) q.push(node->right);
			}
			++depth;
		}
	
		return depth;
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

	// reference answer: breadth-first traversal, no recursion
	int expected_depth(const TreeNode* root) {
		if (root == nullptr) {
			return 0;
		}
		int depth{0};
		std::vector<const TreeNode*> current{root};
		while (!current.empty()) {
			++depth;
			std::vector<const TreeNode*> next;
			for (const TreeNode* node : current) {
				if (node->left != nullptr) next.push_back(node->left);
				if (node->right != nullptr) next.push_back(node->right);
			}
			current = std::move(next);
		}
		return depth;
	}

	void test_depth(const std::string& label,
					const std::vector<int>& level,
					int expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			max_depth(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	void test_big(const std::string& label, const TreeNode* root) {
		const int expected{expected_depth(root)};

		const auto start{std::chrono::steady_clock::now()};
		const int result{max_depth(root)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	// a chain: every node has one child, on the left or on the right
	TreeNode* make_chain(std::vector<TreeNode>& pool, int n, bool to_left) {
		pool.assign(static_cast<std::size_t>(n), TreeNode{});
		for (std::size_t i{0}; i < pool.size(); ++i) {
			pool[i].val = static_cast<int>(i % 100);
			if (i + 1 < pool.size()) {
				(to_left ? pool[i].left : pool[i].right) = &pool[i + 1];
			}
		}
		return &pool.front();
	}

	// a chain that turns left, right, left, right, ...
	TreeNode* make_zigzag(std::vector<TreeNode>& pool, int n) {
		pool.assign(static_cast<std::size_t>(n), TreeNode{});
		for (std::size_t i{0}; i < pool.size(); ++i) {
			if (i + 1 < pool.size()) {
				(i % 2 == 0 ? pool[i].left : pool[i].right) = &pool[i + 1];
			}
		}
		return &pool.front();
	}

	// the node i has the children 2i + 1 and 2i + 2
	TreeNode* make_complete(std::vector<TreeNode>& pool, int n) {
		pool.assign(static_cast<std::size_t>(n), TreeNode{});
		for (std::size_t i{0}; i < pool.size(); ++i) {
			if (2 * i + 1 < pool.size()) pool[i].left = &pool[2 * i + 1];
			if (2 * i + 2 < pool.size()) pool[i].right = &pool[2 * i + 2];
		}
		return &pool.front();
	}

	// every new node is attached to a random free place
	TreeNode* make_random(std::vector<TreeNode>& pool, int n, unsigned seed) {
		pool.assign(static_cast<std::size_t>(n), TreeNode{});
		std::mt19937 gen{seed};
		std::vector<std::pair<std::size_t, bool>> free_places{{0, true}, {0, false}};
		for (std::size_t i{1}; i < pool.size(); ++i) {
			const std::size_t pick{gen() % free_places.size()};
			const auto [parent, is_left] = free_places[pick];
			free_places[pick] = free_places.back();
			free_places.pop_back();
			(is_left ? pool[parent].left : pool[parent].right) = &pool[i];
			free_places.push_back({i, true});
			free_places.push_back({i, false});
		}
		return &pool.front();
	}
}

int main() {
	test_depth("description example 1",
		{3, 9, 20, nil, nil, 15, 7},
		3);
	test_depth("description example 2",
		{1, nil, 2},
		2);
	test_depth("empty tree",
		{},
		0);
	test_depth("single node",
		{5},
		1);
	test_depth("root with left child only",
		{1, 2},
		2);
	test_depth("root with right child only",
		{1, nil, 2},
		2);
	test_depth("root with two leaves",
		{1, 2, 3},
		2);
	test_depth("left chain of 4",
		{1, 2, nil, 3, nil, 4},
		4);
	test_depth("right chain of 4",
		{1, nil, 2, nil, 3, nil, 4},
		4);
	test_depth("zigzag chain",
		{1, 2, nil, nil, 3, 4},
		4);
	test_depth("deep side is the right one",
		{1, 2, 3, nil, nil, nil, 4, nil, 5},
		4);
	test_depth("deep side is the left one",
		{1, 2, 3, 4, nil, nil, nil, 5},
		4);
	test_depth("complete tree of 7 nodes",
		{1, 2, 3, 4, 5, 6, 7},
		3);
	test_depth("complete tree of 15 nodes",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		4);
	test_depth("both subtrees deep, left is deeper",
		{1, 2, 3, 4, nil, 5, nil, 6, nil},
		4);
	test_depth("depth equal at both sides",
		{1, 2, 3, 4, nil, nil, 5},
		3);
	test_depth("deepest leaf in the middle",
		{1, 2, 3, nil, 4, 5, nil, nil, nil, 6},
		4);
	test_depth("negative values",
		{-1, -2, -3, nil, -4},
		3);
	test_depth("zero values",
		{0, 0, 0, nil, 0},
		3);
	test_depth("extreme values",
		{100, -100, 100, nil, -100},
		3);
	test_depth("long left chain with a short right child",
		{1, 2, 3, 4, nil, nil, nil, 5, nil, 6},
		5);
	test_depth("random 1, n=6",
		{-63, -70, -81, nil, 67, nil, 92, -16},
		4);
	test_depth("random 2, n=9",
		{-38, -34, -54, 45, nil, nil, -90, -75, nil, -91, nil, nil, nil, 53, nil, nil, -69},
		6);
	test_depth("random 3, n=12",
		{-92, -63, 18, -59, -46, nil, nil, 59, 13, nil, -79, nil, -85, -61, -82, nil, nil, nil, nil, -98},
		6);
	test_depth("random 4, n=17",
		{-69, -77, -51, 48, -38, 51, -78, 56, nil, 67, -22, -74, nil, 44, nil, nil, nil, nil, -58, nil, nil, 62, -82, nil, nil, nil, nil, nil, nil, 67, -98},
		6);
	test_depth("random 5, n=25",
		{31, nil, -66, 28, -9, -90, 98, 48, -60, 17, -98, -56, 43, -31, 9, nil, nil, nil, nil, nil, -93, nil, nil, -33, nil, -11, 35, 16, nil, nil, nil, nil, nil, -91, 78, nil, -55, -58, nil, nil, nil, nil, nil, -58, 24},
		8);
	test_depth("random 6, n=40",
		{55, 39, -38, -78, 6, nil, -6, 89, -38, 68, 66, -59, nil, 32, -49, -70, nil, nil, 32, 72, -18, nil, 20, -33, 11, nil, 75, nil, nil, -18, nil, 20, 79, -86, 63, 15, nil, nil, nil, 97, nil, -31, nil, 58, nil, 67, 51, nil, nil, 99, -88, nil, nil, nil, nil, 1, nil, nil, nil, nil, nil, nil, nil, nil, -19, 71, 28, nil, nil, 72, 100},
		9);
	test_depth("random 7, n=60",
		{-76, 47, -62, 55, -40, 86, -46, 8, 37, 91, 27, 80, 53, -1, -82, 67, 76, 33, 58, -50, nil, nil, 46, 79, 13, -84, 73, -50, -79, -22, 23, 85, -90, nil, nil, nil, -21, 2, nil, nil, nil, -41, 70, nil, nil, -53, -31, 47, 27, nil, -28, nil, 81, nil, -9, nil, nil, -75, -95, nil, 41, nil, nil, 28, nil, nil, nil, -68, nil, nil, 1, nil, nil, -15, nil, -69, 10, nil, 67, nil, nil, nil, nil, -15, -90, -81, nil, -79, nil, nil, nil, nil, nil, nil, nil, nil, nil, 76, nil, nil, nil, nil, nil, nil, -22, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, -8, nil, -7},
		10);
	test_depth("random 8, n=80",
		{68, 23, 52, -29, 39, -30, -66, 65, 86, -10, -98, 84, -44, 23, 13, 34, nil, 53, -56, 42, -85, 25, nil, -62, 49, -62, nil, 35, -19, -33, -14, nil, nil, 14, 52, nil, nil, nil, -96, nil, -74, nil, nil, -35, nil, -47, -74, -86, 63, 29, nil, 63, 54, -19, 28, -6, -57, nil, nil, nil, nil, nil, nil, nil, nil, -46, nil, 57, nil, nil, 56, nil, 55, nil, nil, 9, 96, nil, -42, 16, 78, nil, nil, nil, nil, 87, 87, nil, -3, nil, nil, 25, 41, nil, nil, nil, 36, 6, -67, nil, 72, 9, 95, 22, nil, nil, -41, nil, nil, nil, 83, nil, nil, nil, nil, nil, nil, nil, nil, -92, 46, nil, nil, 65, 77, nil, -56, nil, -24, nil, -19, nil, nil, nil, nil, nil, 62, nil, nil, nil, nil, 45, nil, nil, nil, 74, nil, nil, nil, -64, 23, nil, 84},
		11);

	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, chain to the left", make_chain(pool, 100000, true));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, chain to the right", make_chain(pool, 100000, false));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, zigzag chain", make_zigzag(pool, 100000));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, complete tree", make_complete(pool, 100000));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, random shape", make_random(pool, 100000, 1));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 100000, random shape, other seed", make_random(pool, 100000, 2));
	}
	{
		std::vector<TreeNode> pool;
		test_big("n = 1, single node", make_chain(pool, 1, true));
	}

	return 0;
}
```