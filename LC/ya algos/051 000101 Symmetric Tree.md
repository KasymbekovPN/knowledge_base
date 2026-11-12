Готовлю задачу 3 блока 7: Symmetric Tree (LC 101). Шаблон собран и проверен прогонами (заглушка, два эталона и семь неверных вариантов), оформляю текст.

Блок 7 (деревья), задача 3: **Symmetric Tree** (LC 101). Это продолжение переворота дерева: тот же приём «зеркальная пара» теперь используется для сравнения.

**Условие.** Дан корень бинарного дерева `root` (может быть `nullptr`). Проверьте, является ли дерево зеркально симметричным относительно своего центра: левое поддерево должно быть зеркальным отражением правого (по структуре и по значениям). Пустое дерево считается симметричным.

**Сигнатура** (уже стоит в шаблоне как заглушка):

```cpp
bool is_symmetric(const TreeNode* root);
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

1. Какая сложность по времени и по памяти?
2. Чем «симметрично» отличается от «левое поддерево равно правому»? Приведите пример, где эти две проверки расходятся.
3. Можно ли решить итеративно? Какие пары нужно класть в очередь или на стек, и в каком порядке?
4. Почему нельзя ограничиться проверкой, что значения на каждом уровне образуют палиндром? Приведите контрпример.

**Цель:** O(n) по времени, память O(h) для рекурсии или O(w) для обхода в ширину. Любое из двух решений годится, но интервьюер попросит обсудить оба.

**Время:** 15 минут. Критерий «выучено»: ≤25 минут, компиляция с первого раза, ни одного предупреждения.

Дерево строится из уровневого списка (`nil` обозначает отсутствующего потомка), узлы лежат в `std::vector<TreeNode>`. Большие тесты строят симметричное дерево из двух зеркальных половин по 49 999 узлов (полная форма, две цепочки, зигзаг, случайная форма) и портят одну деталь на самом глубоком уровне.

```cpp
namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	bool is_symmetric([[maybe_unused]] const TreeNode* root) {
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

	// reference answer: a tree is symmetric exactly when it equals its mirror image,
	// and the level-order lists with `nil`s identify a tree uniquely
	bool expected_symmetric(const TreeNode* root) {
		return level_order(root, false) == level_order(root, true);
	}

	void test_symmetric(const std::string& label,
						const std::vector<int>& level,
						bool expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			is_symmetric(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	void test_big(const std::string& label, const TreeNode* root) {
		const bool expected{expected_symmetric(root)};

		const auto start{std::chrono::steady_clock::now()};
		const bool result{is_symmetric(root)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// the shape of one half: children of the node i (indices are local, -1 means none)
	struct Half {
		std::vector<int> left;
		std::vector<int> right;
	};

	Half make_half(Shape shape, int k, unsigned seed) {
		Half half{std::vector<int>(static_cast<std::size_t>(k), -1),
				  std::vector<int>(static_cast<std::size_t>(k), -1)};
		std::mt19937 gen{seed};
		std::vector<std::pair<int, bool>> free_places{{0, true}, {0, false}};
		for (int i{0}; i + 1 < k; ++i) {
			const std::size_t a{static_cast<std::size_t>(i)};
			switch (shape) {
				case Shape::complete:
					if (2 * i + 1 < k) half.left[a] = 2 * i + 1;
					if (2 * i + 2 < k) half.right[a] = 2 * i + 2;
					break;
				case Shape::left_chain:
					half.left[a] = i + 1;
					break;
				case Shape::right_chain:
					half.right[a] = i + 1;
					break;
				case Shape::zigzag:
					(i % 2 == 0 ? half.left : half.right)[a] = i + 1;
					break;
				case Shape::random:
					break;
			}
		}
		if (shape == Shape::random) {
			for (int i{1}; i < k; ++i) {
				const std::size_t pick{gen() % free_places.size()};
				const auto [parent, is_left] = free_places[pick];
				free_places[pick] = free_places.back();
				free_places.pop_back();
				(is_left ? half.left : half.right)[static_cast<std::size_t>(parent)] = i;
				free_places.push_back({i, true});
				free_places.push_back({i, false});
			}
		}
		return half;
	}

	// root, then a half in nodes 1..k, then its mirror image in nodes k+1..2k; symmetric by construction
	TreeNode* make_symmetric(std::vector<TreeNode>& pool, Shape shape, int k, unsigned seed) {
		const Half half{make_half(shape, k, seed)};
		const std::size_t count{static_cast<std::size_t>(k)};
		pool.assign(2 * count + 1, TreeNode{});
		for (std::size_t i{0}; i < count; ++i) {
			const int value{static_cast<int>((i * 7) % 100) - 50};
			TreeNode& in_left{pool[1 + i]};
			TreeNode& in_right{pool[1 + count + i]};
			in_left.val = value;
			in_right.val = value;
			if (half.left[i] >= 0) {
				in_left.left = &pool[1 + static_cast<std::size_t>(half.left[i])];
				in_right.right = &pool[1 + count + static_cast<std::size_t>(half.left[i])];
			}
			if (half.right[i] >= 0) {
				in_left.right = &pool[1 + static_cast<std::size_t>(half.right[i])];
				in_right.left = &pool[1 + count + static_cast<std::size_t>(half.right[i])];
			}
		}
		pool[0].val = 1;
		pool[0].left = &pool[1];
		pool[0].right = &pool[1 + count];
		return &pool.front();
	}

	// the last node of the right half is a leaf in every shape above
	void change_last_value(std::vector<TreeNode>& pool) {
		pool.back().val += 1;
	}

	void cut_last_leaf(std::vector<TreeNode>& pool) {
		TreeNode* leaf{&pool.back()};
		for (TreeNode& node : pool) {
			if (node.left == leaf) node.left = nullptr;
			if (node.right == leaf) node.right = nullptr;
		}
	}
}

int main() {
	test_symmetric("description example 1, symmetric",
		{1, 2, 2, 3, 4, 4, 3},
		true);
	test_symmetric("description example 2, not symmetric",
		{1, 2, 2, nil, 3, nil, 3},
		false);
	test_symmetric("empty tree",
		{},
		true);
	test_symmetric("single node",
		{5},
		true);
	test_symmetric("root with two equal leaves",
		{1, 2, 2},
		true);
	test_symmetric("root with two different leaves",
		{1, 2, 3},
		false);
	test_symmetric("root with a left child only",
		{1, 2},
		false);
	test_symmetric("root with a right child only",
		{1, nil, 2},
		false);
	test_symmetric("equal children, only the left one has a child",
		{1, 2, 2, 3},
		false);
	test_symmetric("equal children, only the right one has a child",
		{1, 2, 2, nil, nil, 3},
		false);
	test_symmetric("same shape, copy of the left subtree on the right (not a mirror)",
		{1, 2, 2, 3, nil, 3, nil},
		false);
	test_symmetric("mirror shape, values differ deep inside",
		{1, 2, 2, 3, 4, 4, 5},
		false);
	test_symmetric("mirror shape, root children differ",
		{1, 2, 3, 3, 4, 4, 3},
		false);
	test_symmetric("mirror, outer pair differs",
		{1, 2, 2, 3, 4, 4, 6},
		false);
	test_symmetric("mirror, inner pair differs",
		{1, 2, 2, 3, 4, 5, 3},
		false);
	test_symmetric("mirror with only outer grandchildren",
		{1, 2, 2, 3, nil, nil, 3},
		true);
	test_symmetric("mirror with only inner grandchildren",
		{1, 2, 2, nil, 3, 3},
		true);
	test_symmetric("same shape of the left leaning chain on both sides",
		{1, 2, 2, 3, nil, 3},
		false);
	test_symmetric("a deep mirror chain",
		{1, 2, 2, nil, 3, 3, nil, 4, nil, nil, 4},
		true);
	test_symmetric("equal values everywhere, symmetric",
		{7, 7, 7, 7, 7, 7, 7},
		true);
	test_symmetric("equal values everywhere, outer grandchildren only",
		{7, 7, 7, 7, nil, nil, 7},
		true);
	test_symmetric("equal values everywhere, same shape on both sides",
		{7, 7, 7, 7, nil, 7},
		false);
	test_symmetric("negative values, symmetric",
		{-1, -2, -2, -3, -4, -4, -3},
		true);
	test_symmetric("negative values, not symmetric",
		{-1, -2, -2, -3, -4, -3, -4},
		false);
	test_symmetric("zero values, symmetric",
		{0, 0, 0, 0, 0, 0, 0},
		true);
	test_symmetric("extreme values, symmetric",
		{100, -100, -100, 100, -100, -100, 100},
		true);
	test_symmetric("extreme values, not symmetric",
		{100, -100, 100, 100, -100, -100, 100},
		false);
	test_symmetric("complete tree of 15 nodes, symmetric",
		{1, 2, 2, 3, 4, 4, 3, 5, 6, 7, 8, 8, 7, 6, 5},
		true);
	test_symmetric("complete tree of 15 nodes, last pair swapped",
		{1, 2, 2, 3, 4, 4, 3, 5, 6, 7, 8, 8, 7, 5, 6},
		false);
	test_symmetric("random 1, n~5, symmetric",
		{0, 4, 4, 3, nil, nil, 3},
		true);
	test_symmetric("random 2, n~7, symmetric",
		{4, 2, 2, 5, nil, nil, 5, nil, -2, -2},
		true);
	test_symmetric("random 3, n~9, perturbed",
		{-4, -2, -1, nil, 2, 2, nil, nil, -2, -1, nil, 2, nil, nil, 2},
		false);
	test_symmetric("random 4, n~11, symmetric",
		{-2, -2, -2, -4, nil, nil, -4, 3, nil, nil, 3, -2, nil, nil, -2, 0, nil, nil, 0},
		true);
	test_symmetric("random 5, n~13, perturbed",
		{4, 5, 5, -4, 1, 1, -4, -2, 2, 5, nil, nil, nil, 2, -2},
		false);
	test_symmetric("random 6, n~17, symmetric",
		{-4, 0, 0, nil, 1, 1, nil, -2, nil, nil, -2, nil, -4, -4, nil, 5, -5, -5, 5, 3, nil, nil, 2, 2, nil, nil, 3},
		true);
	test_symmetric("random 7, n~21, perturbed",
		{4, 0, 1, 0, -1, 0, 0, 1, 0, -3, -3, -3, -3, 0, 1, nil, nil, nil, -1, nil, nil, nil, nil, nil, nil, nil, nil, -1, nil, nil, nil, 5, 5, 5, 5},
		false);
	test_symmetric("random 8, n~25, perturbed",
		{2, -3, -3, -4, 3, 3, -4, nil, 5, -3, -5, -5, -3, 5, nil, nil, nil, nil, 5, 0, 2, 2, 0, 5, nil, nil, nil, 5, nil, -2, nil, nil, nil, nil, nil, nil, -2, nil, nil, nil, nil, nil, 3, 3},
		false);
	test_symmetric("random 9, n~31, symmetric",
		{-1, -4, -4, -1, -5, -5, -1, 1, nil, -5, 3, 3, -5, nil, 1, 3, nil, -4, 2, -1, 0, 0, -1, 2, -4, nil, 3, 0, nil, nil, nil, nil, nil, nil, -4, 1, -3, -3, 1, -4, nil, nil, nil, nil, nil, nil, 0},
		true);
	test_symmetric("random 10, n~41, perturbed",
		{0, 5, 5, -5, 5, 5, -5, -3, nil, -2, 4, 4, -2, nil, nil, nil, nil, -4, nil, 5, -1, -1, 5, nil, -4, nil, nil, -5, 3, 5, 0, 0, 5, 3, -5, nil, nil, nil, 0, 3, nil, -3, nil, 1, nil, nil, 1, nil, -3, nil, 3, 0, nil, nil, 0, -3, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, -3, 0, nil, nil, 0, nil, nil, nil, nil, 0},
		false);

	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::complete, 49999, 0)};
		test_big("n = 99999, complete halves, symmetric", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::complete, 49999, 0)};
		change_last_value(pool);
		test_big("n = 99999, complete halves, last value differs", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::complete, 49999, 0)};
		cut_last_leaf(pool);
		test_big("n = 99998, complete halves, last leaf is missing", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::left_chain, 49999, 0)};
		test_big("n = 99999, two mirrored chains, symmetric", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::left_chain, 49999, 0)};
		change_last_value(pool);
		test_big("n = 99999, two mirrored chains, the deepest values differ", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::zigzag, 49999, 0)};
		test_big("n = 99999, two mirrored zigzags, symmetric", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::random, 49999, 1)};
		test_big("n = 99999, random mirrored halves, symmetric", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::random, 49999, 2)};
		change_last_value(pool);
		test_big("n = 99999, random mirrored halves, last value differs", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::random, 49999, 3)};
		cut_last_leaf(pool);
		test_big("n = 99998, random mirrored halves, last leaf is missing", root);
	}
	{
		// the left subtree is copied to the right as is, not mirrored
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::right_chain, 49999, 0)};
		const std::size_t count{49999};
		for (std::size_t i{0}; i < count; ++i) {
			std::swap(pool[1 + count + i].left, pool[1 + count + i].right);
		}
		test_big("n = 99999, the right half is a copy, not a mirror", root);
	}

	return 0;
}
```

Проверка шаблона на реальных прогонах: заглушка (всегда `false`) проходит 27 тестов из 49, то есть все с ответом «не симметрично», а рекурсивный и итеративный эталоны проходят все 49, в том числе под `-fsanitize=address,undefined -D_GLIBCXX_ASSERTIONS`. Семь заведомо неверных вариантов шаблон ловит: сравнение левого и правого поддерева без зеркала, сравнение только значений корневых детей, проверка только одной пары потомков, палиндром значений по уровням, пара с `nullptr` и значением считается равной, пропуск пустого дерева (`root != nullptr &&`). Ожидаемые ответы маленьких тестов посчитаны на Python независимой реализацией, большие сверяются сравнением обходов по уровням дерева и его зеркального отражения.

Пришлите решение, когда будет готово.

---

14:45 - 15:13

---

             `1`                   // before while put it into deque
         /     \
      1        1             // deq{1, 1} => {1} == {1} => +
    /    \  /    \
   1    2       2     1          // deq{1, 2, 2, 1} => {1, 2} {1, 2}
 / \     / \     /  \    / \
3  4  5  6  6  5  4  3

---

```cpp
#include <iostream>
#include <format>
#include <cstddef>
#include <vector>
#include <string>
#include <deque>
#include <queue>
#include <utility>
#include <algorithm>
#include <random>
#include <chrono>

namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	bool is_symmetric(const TreeNode* root) {
		if (root == nullptr) return true;

		std::deque<const TreeNode*> deq{};
		deq.push_back(root);
		bool cont{true};
		while (cont) {
			cont = false;
			const std::size_t size{deq.size()};
			
			for (std::size_t i{0}; i < size / 2; ++i) {
				const TreeNode* l{deq[i]};
				const TreeNode* r{deq[size - i - 1]};
				if (!l && !r) {
					continue;
				} else (l && r) {
					if (deq[i]->val != deq[size - i - 1]->val) {
						return false;
					}
				} else {
					return false;
				}
			}
			
			for (std::size_t i{0}; i < size; ++i) {
				const TreeNode* node{deq.front()};
				deq.pop_front();
				
				deq.push_back(node == nullptr ? nullptr : node->left);
				deq.push_back(node == nullptr ? nullptr : node->right);
				
				if (node != nullptr && (node->left != nullptr || node->right != nullptr)) {
					cont = true;
				}
			}
		}
	
		return true;
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

	// reference answer: a tree is symmetric exactly when it equals its mirror image,
	// and the level-order lists with `nil`s identify a tree uniquely
	bool expected_symmetric(const TreeNode* root) {
		return level_order(root, false) == level_order(root, true);
	}

	void test_symmetric(const std::string& label,
						const std::vector<int>& level,
						bool expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			is_symmetric(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	void test_big(const std::string& label, const TreeNode* root) {
		const bool expected{expected_symmetric(root)};

		const auto start{std::chrono::steady_clock::now()};
		const bool result{is_symmetric(root)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// the shape of one half: children of the node i (indices are local, -1 means none)
	struct Half {
		std::vector<int> left;
		std::vector<int> right;
	};

	Half make_half(Shape shape, int k, unsigned seed) {
		Half half{std::vector<int>(static_cast<std::size_t>(k), -1),
				  std::vector<int>(static_cast<std::size_t>(k), -1)};
		std::mt19937 gen{seed};
		std::vector<std::pair<int, bool>> free_places{{0, true}, {0, false}};
		for (int i{0}; i + 1 < k; ++i) {
			const std::size_t a{static_cast<std::size_t>(i)};
			switch (shape) {
				case Shape::complete:
					if (2 * i + 1 < k) half.left[a] = 2 * i + 1;
					if (2 * i + 2 < k) half.right[a] = 2 * i + 2;
					break;
				case Shape::left_chain:
					half.left[a] = i + 1;
					break;
				case Shape::right_chain:
					half.right[a] = i + 1;
					break;
				case Shape::zigzag:
					(i % 2 == 0 ? half.left : half.right)[a] = i + 1;
					break;
				case Shape::random:
					break;
			}
		}
		if (shape == Shape::random) {
			for (int i{1}; i < k; ++i) {
				const std::size_t pick{gen() % free_places.size()};
				const auto [parent, is_left] = free_places[pick];
				free_places[pick] = free_places.back();
				free_places.pop_back();
				(is_left ? half.left : half.right)[static_cast<std::size_t>(parent)] = i;
				free_places.push_back({i, true});
				free_places.push_back({i, false});
			}
		}
		return half;
	}

	// root, then a half in nodes 1..k, then its mirror image in nodes k+1..2k; symmetric by construction
	TreeNode* make_symmetric(std::vector<TreeNode>& pool, Shape shape, int k, unsigned seed) {
		const Half half{make_half(shape, k, seed)};
		const std::size_t count{static_cast<std::size_t>(k)};
		pool.assign(2 * count + 1, TreeNode{});
		for (std::size_t i{0}; i < count; ++i) {
			const int value{static_cast<int>((i * 7) % 100) - 50};
			TreeNode& in_left{pool[1 + i]};
			TreeNode& in_right{pool[1 + count + i]};
			in_left.val = value;
			in_right.val = value;
			if (half.left[i] >= 0) {
				in_left.left = &pool[1 + static_cast<std::size_t>(half.left[i])];
				in_right.right = &pool[1 + count + static_cast<std::size_t>(half.left[i])];
			}
			if (half.right[i] >= 0) {
				in_left.right = &pool[1 + static_cast<std::size_t>(half.right[i])];
				in_right.left = &pool[1 + count + static_cast<std::size_t>(half.right[i])];
			}
		}
		pool[0].val = 1;
		pool[0].left = &pool[1];
		pool[0].right = &pool[1 + count];
		return &pool.front();
	}

	// the last node of the right half is a leaf in every shape above
	void change_last_value(std::vector<TreeNode>& pool) {
		pool.back().val += 1;
	}

	void cut_last_leaf(std::vector<TreeNode>& pool) {
		TreeNode* leaf{&pool.back()};
		for (TreeNode& node : pool) {
			if (node.left == leaf) node.left = nullptr;
			if (node.right == leaf) node.right = nullptr;
		}
	}
}

int main() {
	test_symmetric("description example 1, symmetric",
		{1, 2, 2, 3, 4, 4, 3},
		true);
	test_symmetric("description example 2, not symmetric",
		{1, 2, 2, nil, 3, nil, 3},
		false);
	test_symmetric("empty tree",
		{},
		true);
	test_symmetric("single node",
		{5},
		true);
	test_symmetric("root with two equal leaves",
		{1, 2, 2},
		true);
	test_symmetric("root with two different leaves",
		{1, 2, 3},
		false);
	test_symmetric("root with a left child only",
		{1, 2},
		false);
	test_symmetric("root with a right child only",
		{1, nil, 2},
		false);
	test_symmetric("equal children, only the left one has a child",
		{1, 2, 2, 3},
		false);
	test_symmetric("equal children, only the right one has a child",
		{1, 2, 2, nil, nil, 3},
		false);
	test_symmetric("same shape, copy of the left subtree on the right (not a mirror)",
		{1, 2, 2, 3, nil, 3, nil},
		false);
	test_symmetric("mirror shape, values differ deep inside",
		{1, 2, 2, 3, 4, 4, 5},
		false);
	test_symmetric("mirror shape, root children differ",
		{1, 2, 3, 3, 4, 4, 3},
		false);
	test_symmetric("mirror, outer pair differs",
		{1, 2, 2, 3, 4, 4, 6},
		false);
	test_symmetric("mirror, inner pair differs",
		{1, 2, 2, 3, 4, 5, 3},
		false);
	test_symmetric("mirror with only outer grandchildren",
		{1, 2, 2, 3, nil, nil, 3},
		true);
	test_symmetric("mirror with only inner grandchildren",
		{1, 2, 2, nil, 3, 3},
		true);
	test_symmetric("same shape of the left leaning chain on both sides",
		{1, 2, 2, 3, nil, 3},
		false);
	test_symmetric("a deep mirror chain",
		{1, 2, 2, nil, 3, 3, nil, 4, nil, nil, 4},
		true);
	test_symmetric("equal values everywhere, symmetric",
		{7, 7, 7, 7, 7, 7, 7},
		true);
	test_symmetric("equal values everywhere, outer grandchildren only",
		{7, 7, 7, 7, nil, nil, 7},
		true);
	test_symmetric("equal values everywhere, same shape on both sides",
		{7, 7, 7, 7, nil, 7},
		false);
	test_symmetric("negative values, symmetric",
		{-1, -2, -2, -3, -4, -4, -3},
		true);
	test_symmetric("negative values, not symmetric",
		{-1, -2, -2, -3, -4, -3, -4},
		false);
	test_symmetric("zero values, symmetric",
		{0, 0, 0, 0, 0, 0, 0},
		true);
	test_symmetric("extreme values, symmetric",
		{100, -100, -100, 100, -100, -100, 100},
		true);
	test_symmetric("extreme values, not symmetric",
		{100, -100, 100, 100, -100, -100, 100},
		false);
	test_symmetric("complete tree of 15 nodes, symmetric",
		{1, 2, 2, 3, 4, 4, 3, 5, 6, 7, 8, 8, 7, 6, 5},
		true);
	test_symmetric("complete tree of 15 nodes, last pair swapped",
		{1, 2, 2, 3, 4, 4, 3, 5, 6, 7, 8, 8, 7, 5, 6},
		false);
	test_symmetric("random 1, n~5, symmetric",
		{0, 4, 4, 3, nil, nil, 3},
		true);
	test_symmetric("random 2, n~7, symmetric",
		{4, 2, 2, 5, nil, nil, 5, nil, -2, -2},
		true);
	test_symmetric("random 3, n~9, perturbed",
		{-4, -2, -1, nil, 2, 2, nil, nil, -2, -1, nil, 2, nil, nil, 2},
		false);
	test_symmetric("random 4, n~11, symmetric",
		{-2, -2, -2, -4, nil, nil, -4, 3, nil, nil, 3, -2, nil, nil, -2, 0, nil, nil, 0},
		true);
	test_symmetric("random 5, n~13, perturbed",
		{4, 5, 5, -4, 1, 1, -4, -2, 2, 5, nil, nil, nil, 2, -2},
		false);
	test_symmetric("random 6, n~17, symmetric",
		{-4, 0, 0, nil, 1, 1, nil, -2, nil, nil, -2, nil, -4, -4, nil, 5, -5, -5, 5, 3, nil, nil, 2, 2, nil, nil, 3},
		true);
	test_symmetric("random 7, n~21, perturbed",
		{4, 0, 1, 0, -1, 0, 0, 1, 0, -3, -3, -3, -3, 0, 1, nil, nil, nil, -1, nil, nil, nil, nil, nil, nil, nil, nil, -1, nil, nil, nil, 5, 5, 5, 5},
		false);
	test_symmetric("random 8, n~25, perturbed",
		{2, -3, -3, -4, 3, 3, -4, nil, 5, -3, -5, -5, -3, 5, nil, nil, nil, nil, 5, 0, 2, 2, 0, 5, nil, nil, nil, 5, nil, -2, nil, nil, nil, nil, nil, nil, -2, nil, nil, nil, nil, nil, 3, 3},
		false);
	test_symmetric("random 9, n~31, symmetric",
		{-1, -4, -4, -1, -5, -5, -1, 1, nil, -5, 3, 3, -5, nil, 1, 3, nil, -4, 2, -1, 0, 0, -1, 2, -4, nil, 3, 0, nil, nil, nil, nil, nil, nil, -4, 1, -3, -3, 1, -4, nil, nil, nil, nil, nil, nil, 0},
		true);
	test_symmetric("random 10, n~41, perturbed",
		{0, 5, 5, -5, 5, 5, -5, -3, nil, -2, 4, 4, -2, nil, nil, nil, nil, -4, nil, 5, -1, -1, 5, nil, -4, nil, nil, -5, 3, 5, 0, 0, 5, 3, -5, nil, nil, nil, 0, 3, nil, -3, nil, 1, nil, nil, 1, nil, -3, nil, 3, 0, nil, nil, 0, -3, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, -3, 0, nil, nil, 0, nil, nil, nil, nil, 0},
		false);

	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::complete, 49999, 0)};
		test_big("n = 99999, complete halves, symmetric", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::complete, 49999, 0)};
		change_last_value(pool);
		test_big("n = 99999, complete halves, last value differs", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::complete, 49999, 0)};
		cut_last_leaf(pool);
		test_big("n = 99998, complete halves, last leaf is missing", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::left_chain, 49999, 0)};
		test_big("n = 99999, two mirrored chains, symmetric", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::left_chain, 49999, 0)};
		change_last_value(pool);
		test_big("n = 99999, two mirrored chains, the deepest values differ", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::zigzag, 49999, 0)};
		test_big("n = 99999, two mirrored zigzags, symmetric", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::random, 49999, 1)};
		test_big("n = 99999, random mirrored halves, symmetric", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::random, 49999, 2)};
		change_last_value(pool);
		test_big("n = 99999, random mirrored halves, last value differs", root);
	}
	{
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::random, 49999, 3)};
		cut_last_leaf(pool);
		test_big("n = 99998, random mirrored halves, last leaf is missing", root);
	}
	{
		// the left subtree is copied to the right as is, not mirrored
		std::vector<TreeNode> pool;
		TreeNode* root{make_symmetric(pool, Shape::right_chain, 49999, 0)};
		const std::size_t count{49999};
		for (std::size_t i{0}; i < count; ++i) {
			std::swap(pool[1 + count + i].left, pool[1 + count + i].right);
		}
		test_big("n = 99999, the right half is a copy, not a mirror", root);
	}

	return 0;
}
```