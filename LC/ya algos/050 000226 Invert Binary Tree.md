Готовлю задачу 2 блока 7: Invert Binary Tree. Шаблон собран и проверен прогонами (заглушка падает, эталоны проходят, восемь неверных вариантов ловятся), сейчас оформляю текст.

Блок 7 (деревья), задача 2: **Invert Binary Tree** (LC 226).

**Условие.** Дан корень бинарного дерева `root` (может быть `nullptr`). Переверните дерево зеркально: у каждого узла поменяйте местами левого и правого потомка. Верните корень перевёрнутого дерева. Дерево нужно менять **на месте**: новые узлы создавать нельзя, значения узлов менять нельзя, корень остаётся тем же.

**Сигнатура** (уже стоит в шаблоне как заглушка):

```cpp
TreeNode* invert_tree(TreeNode* root);
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
2. Что изменится, если дерево вырожденное (цепочка из 100 000 узлов)? Как не зависеть от глубины рекурсии?
3. Нужно ли переворачивать детей до или после рекурсивных вызовов? Есть ли разница?
4. Как проверить, что дерево симметрично (LC 101), используя эту же идею?

**Цель:** O(n) по времени, память O(h) для рекурсии или O(w) для обхода в ширину. Любое из двух решений годится, но интервьюер попросит обсудить оба.

**Время:** 15 минут. Критерий «выучено»: ≤25 минут, компиляция с первого раза, ни одного предупреждения.

Дерево строится из уровневого списка (`nil` обозначает отсутствующего потомка), узлы лежат в `std::vector<TreeNode>`. Тест проверяет три вещи: вернулся тот же корень, все узлы результата принадлежат исходному набору (новых нет, ни один не потерян), порядок обхода по уровням совпадает с зеркальным.

```cpp
namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	TreeNode* invert_tree([[maybe_unused]] TreeNode* root) {
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

	// level-order list of the tree with `nil` for missing children and without trailing `nil`s;
	// with `mirrored` the right child is visited before the left one
	std::vector<int> level_order(const TreeNode* root, bool mirrored) {
		std::vector<int> result;
		if (root == nullptr) {
			return result;
		}
		std::vector<const TreeNode*> queue{root};
		for (std::size_t i{0}; i < queue.size(); ++i) {
			const TreeNode* node{queue[i]};
			if (node == nullptr) {
				result.push_back(nil);
				continue;
			}
			result.push_back(node->val);
			queue.push_back(mirrored ? node->right : node->left);
			queue.push_back(mirrored ? node->left : node->right);
		}
		while (!result.empty() && result.back() == nil) {
			result.pop_back();
		}
		return result;
	}

	// the tree must be inverted in place: the same root, no new nodes, nothing lost
	bool uses_only_nodes_of(const TreeNode* root, const std::vector<TreeNode>& pool) {
		if (root == nullptr) {
			return pool.empty();
		}
		std::size_t count{0};
		std::vector<const TreeNode*> queue{root};
		for (std::size_t i{0}; i < queue.size(); ++i) {
			const TreeNode* node{queue[i]};
			if (node < pool.data() || node >= pool.data() + pool.size()) {
				return false;
			}
			++count;
			if (count > pool.size()) {
				return false;
			}
			if (node->left != nullptr) queue.push_back(node->left);
			if (node->right != nullptr) queue.push_back(node->right);
		}
		return count == pool.size();
	}

	bool inverted_correctly(TreeNode* root,
							const std::vector<TreeNode>& pool,
							const std::vector<int>& expected) {
		TreeNode* result{invert_tree(root)};
		return result == root
			&& uses_only_nodes_of(result, pool)
			&& level_order(result, false) == expected;
	}

	void test_invert(const std::string& label,
					 const std::vector<int>& level,
					 const std::vector<int>& expected) {
		std::vector<TreeNode> pool;
		TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			inverted_correctly(root, pool, expected) ? "OK" : "FAIL") << std::flush;
	}

	void test_big(const std::string& label,
				  TreeNode* root,
				  const std::vector<TreeNode>& pool) {
		const std::vector<int> expected{level_order(root, true)};

		const auto start{std::chrono::steady_clock::now()};
		const bool ok{inverted_correctly(root, pool, expected)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			ok ? "OK" : "FAIL",
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
	test_invert("description example 1",
		{4, 2, 7, 1, 3, 6, 9},
		{4, 7, 2, 9, 6, 3, 1});
	test_invert("description example 2",
		{2, 1, 3},
		{2, 3, 1});
	test_invert("description example 3, empty tree",
		{},
		{});
	test_invert("single node",
		{5},
		{5});
	test_invert("root with left child only",
		{1, 2},
		{1, nil, 2});
	test_invert("root with right child only",
		{1, nil, 2},
		{1, 2});
	test_invert("root with two leaves",
		{1, 2, 3},
		{1, 3, 2});
	test_invert("left chain of 4",
		{1, 2, nil, 3, nil, 4},
		{1, nil, 2, nil, 3, nil, 4});
	test_invert("right chain of 4",
		{1, nil, 2, nil, 3, nil, 4},
		{1, 2, nil, 3, nil, 4});
	test_invert("zigzag chain",
		{1, 2, nil, nil, 3, 4},
		{1, nil, 2, 3, nil, nil, 4});
	test_invert("left subtree only has children",
		{1, 2, 3, 4, 5},
		{1, 3, 2, nil, nil, 5, 4});
	test_invert("right subtree only has children",
		{1, 2, 3, nil, nil, 4, 5},
		{1, 3, 2, 5, 4});
	test_invert("complete tree of 7 nodes",
		{1, 2, 3, 4, 5, 6, 7},
		{1, 3, 2, 7, 6, 5, 4});
	test_invert("complete tree of 15 nodes",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		{1, 3, 2, 7, 6, 5, 4, 15, 14, 13, 12, 11, 10, 9, 8});
	test_invert("a node with only a left child inside",
		{1, 2, 3, 4, nil, 5, 6},
		{1, 3, 2, 6, 5, nil, 4});
	test_invert("a node with only a right child inside",
		{1, 2, 3, nil, 4, 5, 6},
		{1, 3, 2, 6, 5, 4});
	test_invert("symmetric tree stays the same shape",
		{1, 2, 2, 3, 4, 4, 3},
		{1, 2, 2, 3, 4, 4, 3});
	test_invert("equal values everywhere",
		{7, 7, 7, 7, nil, 7, 7},
		{7, 7, 7, 7, 7, nil, 7});
	test_invert("negative values",
		{-1, -2, -3, nil, -4},
		{-1, -3, -2, nil, nil, -4});
	test_invert("zero values",
		{0, 0, 0, nil, 0},
		{0, 0, 0, nil, nil, 0});
	test_invert("extreme values",
		{100, -100, 100, nil, -100},
		{100, 100, -100, nil, nil, -100});
	test_invert("long left chain with a short right child",
		{1, 2, 3, 4, nil, nil, nil, 5, nil, 6},
		{1, 3, 2, nil, nil, nil, 4, nil, 5, nil, 6});
	test_invert("random 1, n=6",
		{-13, -2, 40, 16, nil, 78, nil, nil, nil, 42},
		{-13, 40, -2, nil, 78, nil, 16, nil, 42});
	test_invert("random 2, n=9",
		{83, 39, -9, nil, -17, 9, 42, -78, nil, nil, nil, nil, 9, nil, nil, nil, -64},
		{83, -9, 39, 42, 9, -17, nil, 9, nil, nil, nil, nil, -78, -64});
	test_invert("random 3, n=12",
		{-4, -50, 94, -91, 37, 68, 94, nil, nil, 22, 32, -37, nil, nil, nil, nil, -70, -44},
		{-4, 94, -50, 94, 68, 37, -91, nil, nil, nil, -37, 32, 22, nil, nil, nil, nil, nil, -44, -70});
	test_invert("random 4, n=17",
		{4, -74, -93, 60, -15, 76, -60, nil, nil, nil, nil, 66, 28, nil, -51, nil, 46, -51, 59, nil, -43, nil, nil, nil, nil, 76, -80, nil, nil, nil, -80},
		{4, -93, -74, -60, 76, -15, 60, -51, nil, 28, 66, nil, nil, nil, nil, -43, nil, 59, -51, 46, nil, nil, nil, -80, 76, nil, nil, nil, nil, nil, nil, -80});
	test_invert("random 5, n=25",
		{-65, -52, -74, nil, -68, -72, -98, -80, -85, 58, -43, -33, nil, nil, nil, -22, nil, 40, -33, -38, -82, nil, nil, nil, nil, nil, nil, 29, 78, nil, 32, 54, nil, nil, -67, nil, -62, nil, nil, 66, 65, 90},
		{-65, -74, -52, -98, -72, -68, nil, nil, -33, -43, 58, -85, -80, nil, nil, -82, -38, -33, 40, nil, -22, nil, nil, nil, 54, 32, nil, 78, 29, nil, nil, nil, nil, 65, 66, nil, nil, -62, nil, -67, nil, nil, nil, nil, nil, nil, nil, nil, 90});
	test_invert("random 6, n=40",
		{39, 30, -71, -100, -47, 65, -15, -72, nil, nil, -56, -53, 18, 13, -57, nil, nil, nil, nil, -39, -18, 10, nil, 75, -63, nil, nil, -70, -48, -39, nil, nil, nil, nil, -39, nil, -76, 33, -10, nil, 60, 3, -2, nil, nil, nil, nil, nil, -72, -2, -69, 44, nil, nil, nil, nil, nil, -7, nil, nil, nil, nil, -10, nil, nil, -78, 76, 10, nil, nil, nil, 84, nil, nil, 42, nil, nil, -41},
		{39, -71, 30, -15, 65, -47, -100, -57, 13, 18, -53, -56, nil, nil, -72, nil, nil, -63, 75, nil, 10, -18, -39, nil, nil, nil, nil, -76, nil, -39, nil, nil, nil, nil, -39, -48, -70, nil, nil, nil, nil, -2, 3, 60, nil, -10, 33, nil, nil, nil, nil, nil, 44, -69, -2, -72, nil, nil, nil, -10, nil, nil, nil, nil, -7, nil, 10, 76, -78, 42, nil, nil, 84, nil, nil, nil, -41});

	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_chain(pool, 100000, true)};
		test_big("n = 100000, chain to the left", root, pool);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_chain(pool, 100000, false)};
		test_big("n = 100000, chain to the right", root, pool);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_zigzag(pool, 100000)};
		test_big("n = 100000, zigzag chain", root, pool);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_complete(pool, 100000)};
		test_big("n = 100000, complete tree", root, pool);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_random(pool, 100000, 1)};
		test_big("n = 100000, random shape", root, pool);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_random(pool, 100000, 2)};
		test_big("n = 100000, random shape, other seed", root, pool);
	}

	return 0;
}
```

Проверка шаблона на реальных прогонах: заглушка проходит 1 тест из 34 (пустое дерево), рекурсивный и итеративный эталоны проходят все 34, в том числе под `-fsanitize=address,undefined -D_GLIBCXX_ASSERTIONS`. Семь заведомо неверных вариантов шаблон ловит: переворот только корня, спуск только по левому поддереву, двойной обмен (дерево остаётся прежним), создание новых узлов, обмен значениями вместо ссылок, потерянный `return root` и пропуск узлов с одним потомком. Ожидаемые зеркальные списки маленьких тестов посчитаны независимой реализацией на Python, большие сверяются обходом уровней в обратном порядке потомков.

Пришлите решение, когда будет готово.

---

work: 22:35 - 22:41

---

---

```cpp
#include <iostream>
#include <format>
#include <vector>
#include <queue>
#include <string>
#include <cstddef>
#include <chrono>
#include <cstddef>

namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	TreeNode* invert_tree(TreeNode* root) {
		if (root == nullptr) return root;
		
		std::queue<TreeNode*> q;
		q.push(root);
		
		while (!q.empty()) {
			const int size{static_cast<int>(q.size())};
			for (int i{0}; i < size; ++i) {
				TreeNode* node{q.front()};
				q.pop();
				
				TreeNode* buffer{node.left};
				node->left = node.right;
				node->right = buffer;
				
				if (node->left != nullptr) q.push(node->left);
				if (node->right != nullptr) q.push(node->right);
			}
		}
	
		return root;
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

	// level-order list of the tree with `nil` for missing children and without trailing `nil`s;
	// with `mirrored` the right child is visited before the left one
	std::vector<int> level_order(const TreeNode* root, bool mirrored) {
		std::vector<int> result;
		if (root == nullptr) {
			return result;
		}
		std::vector<const TreeNode*> queue{root};
		for (std::size_t i{0}; i < queue.size(); ++i) {
			const TreeNode* node{queue[i]};
			if (node == nullptr) {
				result.push_back(nil);
				continue;
			}
			result.push_back(node->val);
			queue.push_back(mirrored ? node->right : node->left);
			queue.push_back(mirrored ? node->left : node->right);
		}
		while (!result.empty() && result.back() == nil) {
			result.pop_back();
		}
		return result;
	}

	// the tree must be inverted in place: the same root, no new nodes, nothing lost
	bool uses_only_nodes_of(const TreeNode* root, const std::vector<TreeNode>& pool) {
		if (root == nullptr) {
			return pool.empty();
		}
		std::size_t count{0};
		std::vector<const TreeNode*> queue{root};
		for (std::size_t i{0}; i < queue.size(); ++i) {
			const TreeNode* node{queue[i]};
			if (node < pool.data() || node >= pool.data() + pool.size()) {
				return false;
			}
			++count;
			if (count > pool.size()) {
				return false;
			}
			if (node->left != nullptr) queue.push_back(node->left);
			if (node->right != nullptr) queue.push_back(node->right);
		}
		return count == pool.size();
	}

	bool inverted_correctly(TreeNode* root,
							const std::vector<TreeNode>& pool,
							const std::vector<int>& expected) {
		TreeNode* result{invert_tree(root)};
		return result == root
			&& uses_only_nodes_of(result, pool)
			&& level_order(result, false) == expected;
	}

	void test_invert(const std::string& label,
					 const std::vector<int>& level,
					 const std::vector<int>& expected) {
		std::vector<TreeNode> pool;
		TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			inverted_correctly(root, pool, expected) ? "OK" : "FAIL") << std::flush;
	}

	void test_big(const std::string& label,
				  TreeNode* root,
				  const std::vector<TreeNode>& pool) {
		const std::vector<int> expected{level_order(root, true)};

		const auto start{std::chrono::steady_clock::now()};
		const bool ok{inverted_correctly(root, pool, expected)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			ok ? "OK" : "FAIL",
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
	test_invert("description example 1",
		{4, 2, 7, 1, 3, 6, 9},
		{4, 7, 2, 9, 6, 3, 1});
	test_invert("description example 2",
		{2, 1, 3},
		{2, 3, 1});
	test_invert("description example 3, empty tree",
		{},
		{});
	test_invert("single node",
		{5},
		{5});
	test_invert("root with left child only",
		{1, 2},
		{1, nil, 2});
	test_invert("root with right child only",
		{1, nil, 2},
		{1, 2});
	test_invert("root with two leaves",
		{1, 2, 3},
		{1, 3, 2});
	test_invert("left chain of 4",
		{1, 2, nil, 3, nil, 4},
		{1, nil, 2, nil, 3, nil, 4});
	test_invert("right chain of 4",
		{1, nil, 2, nil, 3, nil, 4},
		{1, 2, nil, 3, nil, 4});
	test_invert("zigzag chain",
		{1, 2, nil, nil, 3, 4},
		{1, nil, 2, 3, nil, nil, 4});
	test_invert("left subtree only has children",
		{1, 2, 3, 4, 5},
		{1, 3, 2, nil, nil, 5, 4});
	test_invert("right subtree only has children",
		{1, 2, 3, nil, nil, 4, 5},
		{1, 3, 2, 5, 4});
	test_invert("complete tree of 7 nodes",
		{1, 2, 3, 4, 5, 6, 7},
		{1, 3, 2, 7, 6, 5, 4});
	test_invert("complete tree of 15 nodes",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		{1, 3, 2, 7, 6, 5, 4, 15, 14, 13, 12, 11, 10, 9, 8});
	test_invert("a node with only a left child inside",
		{1, 2, 3, 4, nil, 5, 6},
		{1, 3, 2, 6, 5, nil, 4});
	test_invert("a node with only a right child inside",
		{1, 2, 3, nil, 4, 5, 6},
		{1, 3, 2, 6, 5, 4});
	test_invert("symmetric tree stays the same shape",
		{1, 2, 2, 3, 4, 4, 3},
		{1, 2, 2, 3, 4, 4, 3});
	test_invert("equal values everywhere",
		{7, 7, 7, 7, nil, 7, 7},
		{7, 7, 7, 7, 7, nil, 7});
	test_invert("negative values",
		{-1, -2, -3, nil, -4},
		{-1, -3, -2, nil, nil, -4});
	test_invert("zero values",
		{0, 0, 0, nil, 0},
		{0, 0, 0, nil, nil, 0});
	test_invert("extreme values",
		{100, -100, 100, nil, -100},
		{100, 100, -100, nil, nil, -100});
	test_invert("long left chain with a short right child",
		{1, 2, 3, 4, nil, nil, nil, 5, nil, 6},
		{1, 3, 2, nil, nil, nil, 4, nil, 5, nil, 6});
	test_invert("random 1, n=6",
		{-13, -2, 40, 16, nil, 78, nil, nil, nil, 42},
		{-13, 40, -2, nil, 78, nil, 16, nil, 42});
	test_invert("random 2, n=9",
		{83, 39, -9, nil, -17, 9, 42, -78, nil, nil, nil, nil, 9, nil, nil, nil, -64},
		{83, -9, 39, 42, 9, -17, nil, 9, nil, nil, nil, nil, -78, -64});
	test_invert("random 3, n=12",
		{-4, -50, 94, -91, 37, 68, 94, nil, nil, 22, 32, -37, nil, nil, nil, nil, -70, -44},
		{-4, 94, -50, 94, 68, 37, -91, nil, nil, nil, -37, 32, 22, nil, nil, nil, nil, nil, -44, -70});
	test_invert("random 4, n=17",
		{4, -74, -93, 60, -15, 76, -60, nil, nil, nil, nil, 66, 28, nil, -51, nil, 46, -51, 59, nil, -43, nil, nil, nil, nil, 76, -80, nil, nil, nil, -80},
		{4, -93, -74, -60, 76, -15, 60, -51, nil, 28, 66, nil, nil, nil, nil, -43, nil, 59, -51, 46, nil, nil, nil, -80, 76, nil, nil, nil, nil, nil, nil, -80});
	test_invert("random 5, n=25",
		{-65, -52, -74, nil, -68, -72, -98, -80, -85, 58, -43, -33, nil, nil, nil, -22, nil, 40, -33, -38, -82, nil, nil, nil, nil, nil, nil, 29, 78, nil, 32, 54, nil, nil, -67, nil, -62, nil, nil, 66, 65, 90},
		{-65, -74, -52, -98, -72, -68, nil, nil, -33, -43, 58, -85, -80, nil, nil, -82, -38, -33, 40, nil, -22, nil, nil, nil, 54, 32, nil, 78, 29, nil, nil, nil, nil, 65, 66, nil, nil, -62, nil, -67, nil, nil, nil, nil, nil, nil, nil, nil, 90});
	test_invert("random 6, n=40",
		{39, 30, -71, -100, -47, 65, -15, -72, nil, nil, -56, -53, 18, 13, -57, nil, nil, nil, nil, -39, -18, 10, nil, 75, -63, nil, nil, -70, -48, -39, nil, nil, nil, nil, -39, nil, -76, 33, -10, nil, 60, 3, -2, nil, nil, nil, nil, nil, -72, -2, -69, 44, nil, nil, nil, nil, nil, -7, nil, nil, nil, nil, -10, nil, nil, -78, 76, 10, nil, nil, nil, 84, nil, nil, 42, nil, nil, -41},
		{39, -71, 30, -15, 65, -47, -100, -57, 13, 18, -53, -56, nil, nil, -72, nil, nil, -63, 75, nil, 10, -18, -39, nil, nil, nil, nil, -76, nil, -39, nil, nil, nil, nil, -39, -48, -70, nil, nil, nil, nil, -2, 3, 60, nil, -10, 33, nil, nil, nil, nil, nil, 44, -69, -2, -72, nil, nil, nil, -10, nil, nil, nil, nil, -7, nil, 10, 76, -78, 42, nil, nil, 84, nil, nil, nil, -41});

	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_chain(pool, 100000, true)};
		test_big("n = 100000, chain to the left", root, pool);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_chain(pool, 100000, false)};
		test_big("n = 100000, chain to the right", root, pool);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_zigzag(pool, 100000)};
		test_big("n = 100000, zigzag chain", root, pool);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_complete(pool, 100000)};
		test_big("n = 100000, complete tree", root, pool);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_random(pool, 100000, 1)};
		test_big("n = 100000, random shape", root, pool);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_random(pool, 100000, 2)};
		test_big("n = 100000, random shape, other seed", root, pool);
	}

	return 0;
}
```