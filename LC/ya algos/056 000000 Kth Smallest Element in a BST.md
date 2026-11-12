Задача 8 блока 7: Kth Smallest Element in a BST. Собираю тесты и проверяю.

Задача 8 блока 7 готова. Проверка по запуску: заглушка (возвращает 0) даёт 48 FAIL из 48; три эталона (обход in-order в вектор, итеративный in-order с ранней остановкой, рекурсия по размерам поддеревьев) дают верные ответы, первые два проходят все 48 без предупреждений и чисто под ASan/UBSan, а третий на длинных цепочках помечается `(too slow)`, то есть тест на время отличает O(n·h) от O(n). Четыре неверных варианта (на единицу сдвинутый `k`, k-й по убыванию, прямой обход вместо in-order, `k` не уменьшается при уходе вправо) ловятся.

## Kth Smallest Element in a BST

Дано корректное бинарное дерево поиска (строго: слева меньше, справа больше) и число `k`. Верните `k`-е по величине значение в дереве (счёт с единицы).

```cpp
int kth_smallest(const TreeNode* root, int k);
```

**Ограничения:** 1 ≤ k ≤ n ≤ 10⁵, значения лежат во всём диапазоне `int`, дерево непустое и корректное.

**Вопросы интервьюера:**

1. Почему нужен именно in-order обход? Что он даёт для дерева поиска?
2. Какая сложность по времени и по памяти? Можно ли остановиться, не обходя дерево целиком, и что тогда изменится в оценке?
3. Как обойти без рекурсии? Что хранится в стеке?
4. Что делать, если дерево часто меняется, а запросов `k-го` много? Что нужно хранить в узле, чтобы отвечать за O(h)?

**Цель:** O(h + k) по времени, O(h) по памяти (допустимо O(n), но интервьюер спросит, как лучше). **Ориентир по времени:** 12 минут.

```cpp
namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	int kth_smallest([[maybe_unused]] const TreeNode* root, [[maybe_unused]] int k) {
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

	void test_kth(const std::string& label,
				  const std::vector<int>& level,
				  int k,
				  int expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			kth_smallest(root, k) == expected ? "OK" : "FAIL") << std::flush;
	}

	// the tree is made by `make_bst`: the k-th smallest value is known without traversing anything
	void test_big(const std::string& label, const std::vector<TreeNode>& pool, int k) {
		const int expected{static_cast<int>(std::numeric_limits<int>::min() + static_cast<long long>(k - 1) * 20000)};

		const auto start{std::chrono::steady_clock::now()};
		const int result{kth_smallest(&pool.front(), k)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// a search tree of the given shape: the node 0 is the root, the value with the in-order position r (from 0)
	// is `lowest + r * 20000`, so the values are spread over the whole int range
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

	test_kth("description example 1",
		{3, 1, 4, nil, 2},
		1, 1);
	test_kth("description example 2",
		{5, 3, 6, 2, 4, nil, nil, 1},
		3, 3);
	test_kth("single node",
		{7},
		1, 7);
	test_kth("two nodes, the left child, k = 1",
		{2, 1},
		1, 1);
	test_kth("two nodes, the left child, k = 2",
		{2, 1},
		2, 2);
	test_kth("two nodes, the right child, k = 1",
		{1, nil, 2},
		1, 1);
	test_kth("two nodes, the right child, k = 2",
		{1, nil, 2},
		2, 2);
	test_kth("left chain, the smallest is the deepest node",
		{5, 4, nil, 3, nil, 2, nil, 1},
		1, 1);
	test_kth("left chain, the largest is the root",
		{5, 4, nil, 3, nil, 2, nil, 1},
		5, 5);
	test_kth("left chain, k in the middle",
		{5, 4, nil, 3, nil, 2, nil, 1},
		3, 3);
	test_kth("right chain, the smallest is the root",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		1, 1);
	test_kth("right chain, the largest is the deepest node",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		5, 5);
	test_kth("right chain, k in the middle",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		3, 3);
	test_kth("perfect tree of 7 nodes, k = 1",
		{4, 2, 6, 1, 3, 5, 7},
		1, 1);
	test_kth("perfect tree of 7 nodes, k is the root",
		{4, 2, 6, 1, 3, 5, 7},
		4, 4);
	test_kth("perfect tree of 7 nodes, k = 7",
		{4, 2, 6, 1, 3, 5, 7},
		7, 7);
	test_kth("perfect tree of 7 nodes, k = 2",
		{4, 2, 6, 1, 3, 5, 7},
		2, 2);
	test_kth("perfect tree of 7 nodes, k = 6",
		{4, 2, 6, 1, 3, 5, 7},
		6, 6);
	test_kth("the answer is in the right subtree and k exceeds the size of the left one",
		{10, 5, 15, nil, nil, 12, 20, 11, 13},
		7, 20);
	test_kth("the answer is in the right subtree, the left subtree is large",
		{10, 5, 15, 3, 7, 12, nil, 1, 4, 6, 8},
		10, 15);
	test_kth("the answer is the root, the left subtree is large",
		{10, 5, 15, 3, 7, 12, 18, 1, 4, 6, 8},
		8, 10);
	test_kth("the answer is deep in the left subtree of the left subtree",
		{10, 5, 15, 3, 7, 12, 18, 1, 4, 6, 8},
		2, 3);
	test_kth("zigzag, the answer is in the middle",
		{10, 5, nil, nil, 8, 6, nil, nil, 7},
		2, 6);
	test_kth("negative values, zero and positive",
		{0, -5, 5, -10, -3, 3, 10},
		3, -3);
	test_kth("negative values only",
		{-1, -5, nil, -8, -3, nil, nil, nil, -2},
		2, -5);
	test_kth("values at the ends of the range",
		{100, -100, nil, nil, 0, -50, 50},
		4, 50);
	test_kth("random 1, n = 8, k = 8",
		{80, -59, 85, -96, -23, nil, nil, nil, nil, nil, 73, 37, nil, nil, 68},
		8, 85);
	test_kth("random 2, n = 12, k = 3",
		{11, -94, 96, nil, 4, 24, nil, -35, nil, 23, 61, nil, nil, nil, nil, 59, 95, 28, nil, nil, nil, nil, 56},
		3, 4);
	test_kth("random 3, n = 20, k = 2",
		{-100, nil, -11, -52, 75, -94, -20, 71, 81, nil, -56, -21, -16, 22, 73, nil, 97, -68, nil, -27, nil, nil, nil, -1, 44, nil, nil, nil, nil, nil, nil, nil, nil, nil, 15, nil, 46},
		2, -94);
	test_kth("random 4, n = 30, k = 29",
		{-91, -96, -84, -100, nil, nil, -20, nil, nil, -23, 83, -43, nil, 24, nil, -70, -37, 16, 29, -82, -63, nil, -33, 1, nil, nil, 62, nil, -72, nil, -49, nil, -29, -18, 5, 54, 73, nil, nil, nil, nil, nil, nil, nil, -6, nil, 14, nil, 61, nil, nil, nil, nil, 12, nil, nil, nil, 8},
		29, 73);
	test_kth("random 5, n = 40, k = 36",
		{34, -53, 70, -87, -43, 37, 85, -89, -61, -52, -27, nil, 64, 77, 97, -91, nil, -74, -55, nil, nil, -33, 26, 58, 65, nil, 79, 91, nil, nil, nil, nil, -65, nil, nil, nil, nil, 1, 29, 42, nil, nil, 67, nil, 83, nil, nil, nil, -64, -16, 24, nil, nil, 41, 47, nil, nil, nil, nil, nil, nil, -20, -8, 19, nil, nil, nil, nil, 55, nil, nil, -15, nil, nil, nil, nil, 57},
		36, 79);
	test_kth("random 6, n = 50, k = 43",
		{84, -69, 86, -72, -63, 85, 97, -83, -70, nil, 17, nil, nil, 91, 98, -95, -73, nil, nil, -62, 75, nil, nil, nil, 100, -96, nil, -74, nil, nil, -46, 36, 78, nil, nil, nil, nil, nil, nil, -51, 3, 24, 60, 77, 79, -53, -47, -22, 6, nil, 33, 45, 72, nil, nil, nil, 80, -57, nil, nil, nil, -25, -10, 4, 13, 25, nil, nil, nil, 70, nil, nil, 81, -58, nil, -37, -23, nil, 1, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 0, 2},
		43, 81);

	constexpr int n{100000};
	{
		const std::vector<TreeNode> pool{make_bst(Shape::complete, n, 0)};
		test_big("n = 100000, complete tree, k = 1", pool, 1);
		test_big("n = 100000, complete tree, k = n", pool, n);
		test_big("n = 100000, complete tree, k = n / 2", pool, n / 2);
		test_big("n = 100000, complete tree, k = 33333", pool, 33333);
	}
	{
		const std::vector<TreeNode> pool{make_bst(Shape::left_chain, n, 0)};
		test_big("n = 100000, left chain, k = 1 (the deepest node)", pool, 1);
		test_big("n = 100000, left chain, k = n (the root)", pool, n);
		test_big("n = 100000, left chain, k = n / 2", pool, n / 2);
	}
	{
		const std::vector<TreeNode> pool{make_bst(Shape::right_chain, n, 0)};
		test_big("n = 100000, right chain, k = 1 (the root)", pool, 1);
		test_big("n = 100000, right chain, k = n (the deepest node)", pool, n);
		test_big("n = 100000, right chain, k = n / 2", pool, n / 2);
	}
	{
		const std::vector<TreeNode> pool{make_bst(Shape::zigzag, n, 0)};
		test_big("n = 100000, zigzag, k = 1", pool, 1);
		test_big("n = 100000, zigzag, k = n", pool, n);
		test_big("n = 100000, zigzag, k = n / 2", pool, n / 2);
	}
	{
		const std::vector<TreeNode> pool{make_bst(Shape::random, n, 1)};
		test_big("n = 100000, random tree, k = 1", pool, 1);
		test_big("n = 100000, random tree, k = n", pool, n);
		test_big("n = 100000, random tree, k = 77777", pool, 77777);
	}

	return 0;
}
```

Проверка по времени (1 секунда) не умеет прерывать зависшую программу: если решение зациклится, остановите его сами. Рекурсия глубиной 10⁵ нормально работает без санитайзеров, но под `-fsanitize=address` может упасть по стеку из-за размера кадров: это не ошибка алгоритма, увеличьте стек через `ulimit -s unlimited` или проверьте итеративный вариант.

Пришлите решение вложением. Когда будет готово, я его разберу и закончу списком «в журнал ошибок».

---

14:12 - 14:32

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

- при `k = 1` ответ 1 (самый левый узел);
- при `k = 3` ответ 3 (это пример 2 из шаблона);
- при `k = 5` ответ 5 (корень: в левом поддереве 4 узла, значит `k = 4 + 1`);
- при `k = 6` ответ 6 (самый правый узел).


```
        5          // {5}
       / \
      3   6        // {3, 5, 6}
     / \
    2   4          // {2, 3, 4, 5, 6}
   /
  1                // {1, 2, 3, 4, 5, 6}
```

---

```cpp
#include <iostream>
#include <format>
#include <cstddef>
#include <string>
#include <vector>
#include <queue>
#include <tuple>
#include <chrono>
#include <random>

namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	void fill(const TreeNode* node,
			  std::priority_queue<int, std::vector<int>, std::greater<>>& pq) {
		if (node == nullptr) return;
		
		fill(node->left, pq);
		fill(node->right,pq);
		pq.push(node->val);
	}
	
	int kth_smallest(const TreeNode* root, int k) {
		
		std::priority_queue<int, std::vector<int>, std::greater<>> pq;
		fill(root, pq);
	
		while (true) {
			if (--k == 0) { return pq.top(); }
			pq.pop();
		}
	}

	/*
		Идея: спуск влево накапливает стек, очередной узел из стека это следующий по 
		величине, после него идём в правое поддерево. Для k малого и 
		сбалансированного дерева это O(h + k) вместо O(n log n).
	*/
	int kth_smallest1(const TreeNode* root, int k) {
		std::vector<const TreeNode*> st;
		const TreeNode* node{root};

		while (node != nullptr || !st.empty())	 {
			while (node != nullptr) {
				st.push_back(node);
				node = node->left;
			}
			
			node = st.back();
			st.pop_back();
			
			if (--k == 0) return node->val;
			
			node = node->right;
		}

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

	void test_kth(const std::string& label,
				  const std::vector<int>& level,
				  int k,
				  int expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			kth_smallest(root, k) == expected ? "OK" : "FAIL") << std::flush;
	}

	// the tree is made by `make_bst`: the k-th smallest value is known without traversing anything
	void test_big(const std::string& label, const std::vector<TreeNode>& pool, int k) {
		const int expected{static_cast<int>(std::numeric_limits<int>::min() + static_cast<long long>(k - 1) * 20000)};

		const auto start{std::chrono::steady_clock::now()};
		const int result{kth_smallest(&pool.front(), k)};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// a search tree of the given shape: the node 0 is the root, the value with the in-order position r (from 0)
	// is `lowest + r * 20000`, so the values are spread over the whole int range
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

	test_kth("description example 1",
		{3, 1, 4, nil, 2},
		1, 1);
	test_kth("description example 2",
		{5, 3, 6, 2, 4, nil, nil, 1},
		3, 3);
	test_kth("single node",
		{7},
		1, 7);
	test_kth("two nodes, the left child, k = 1",
		{2, 1},
		1, 1);
	test_kth("two nodes, the left child, k = 2",
		{2, 1},
		2, 2);
	test_kth("two nodes, the right child, k = 1",
		{1, nil, 2},
		1, 1);
	test_kth("two nodes, the right child, k = 2",
		{1, nil, 2},
		2, 2);
	test_kth("left chain, the smallest is the deepest node",
		{5, 4, nil, 3, nil, 2, nil, 1},
		1, 1);
	test_kth("left chain, the largest is the root",
		{5, 4, nil, 3, nil, 2, nil, 1},
		5, 5);
	test_kth("left chain, k in the middle",
		{5, 4, nil, 3, nil, 2, nil, 1},
		3, 3);
	test_kth("right chain, the smallest is the root",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		1, 1);
	test_kth("right chain, the largest is the deepest node",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		5, 5);
	test_kth("right chain, k in the middle",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		3, 3);
	test_kth("perfect tree of 7 nodes, k = 1",
		{4, 2, 6, 1, 3, 5, 7},
		1, 1);
	test_kth("perfect tree of 7 nodes, k is the root",
		{4, 2, 6, 1, 3, 5, 7},
		4, 4);
	test_kth("perfect tree of 7 nodes, k = 7",
		{4, 2, 6, 1, 3, 5, 7},
		7, 7);
	test_kth("perfect tree of 7 nodes, k = 2",
		{4, 2, 6, 1, 3, 5, 7},
		2, 2);
	test_kth("perfect tree of 7 nodes, k = 6",
		{4, 2, 6, 1, 3, 5, 7},
		6, 6);
	test_kth("the answer is in the right subtree and k exceeds the size of the left one",
		{10, 5, 15, nil, nil, 12, 20, 11, 13},
		7, 20);
	test_kth("the answer is in the right subtree, the left subtree is large",
		{10, 5, 15, 3, 7, 12, nil, 1, 4, 6, 8},
		10, 15);
	test_kth("the answer is the root, the left subtree is large",
		{10, 5, 15, 3, 7, 12, 18, 1, 4, 6, 8},
		8, 10);
	test_kth("the answer is deep in the left subtree of the left subtree",
		{10, 5, 15, 3, 7, 12, 18, 1, 4, 6, 8},
		2, 3);
	test_kth("zigzag, the answer is in the middle",
		{10, 5, nil, nil, 8, 6, nil, nil, 7},
		2, 6);
	test_kth("negative values, zero and positive",
		{0, -5, 5, -10, -3, 3, 10},
		3, -3);
	test_kth("negative values only",
		{-1, -5, nil, -8, -3, nil, nil, nil, -2},
		2, -5);
	test_kth("values at the ends of the range",
		{100, -100, nil, nil, 0, -50, 50},
		4, 50);
	test_kth("random 1, n = 8, k = 8",
		{80, -59, 85, -96, -23, nil, nil, nil, nil, nil, 73, 37, nil, nil, 68},
		8, 85);
	test_kth("random 2, n = 12, k = 3",
		{11, -94, 96, nil, 4, 24, nil, -35, nil, 23, 61, nil, nil, nil, nil, 59, 95, 28, nil, nil, nil, nil, 56},
		3, 4);
	test_kth("random 3, n = 20, k = 2",
		{-100, nil, -11, -52, 75, -94, -20, 71, 81, nil, -56, -21, -16, 22, 73, nil, 97, -68, nil, -27, nil, nil, nil, -1, 44, nil, nil, nil, nil, nil, nil, nil, nil, nil, 15, nil, 46},
		2, -94);
	test_kth("random 4, n = 30, k = 29",
		{-91, -96, -84, -100, nil, nil, -20, nil, nil, -23, 83, -43, nil, 24, nil, -70, -37, 16, 29, -82, -63, nil, -33, 1, nil, nil, 62, nil, -72, nil, -49, nil, -29, -18, 5, 54, 73, nil, nil, nil, nil, nil, nil, nil, -6, nil, 14, nil, 61, nil, nil, nil, nil, 12, nil, nil, nil, 8},
		29, 73);
	test_kth("random 5, n = 40, k = 36",
		{34, -53, 70, -87, -43, 37, 85, -89, -61, -52, -27, nil, 64, 77, 97, -91, nil, -74, -55, nil, nil, -33, 26, 58, 65, nil, 79, 91, nil, nil, nil, nil, -65, nil, nil, nil, nil, 1, 29, 42, nil, nil, 67, nil, 83, nil, nil, nil, -64, -16, 24, nil, nil, 41, 47, nil, nil, nil, nil, nil, nil, -20, -8, 19, nil, nil, nil, nil, 55, nil, nil, -15, nil, nil, nil, nil, 57},
		36, 79);
	test_kth("random 6, n = 50, k = 43",
		{84, -69, 86, -72, -63, 85, 97, -83, -70, nil, 17, nil, nil, 91, 98, -95, -73, nil, nil, -62, 75, nil, nil, nil, 100, -96, nil, -74, nil, nil, -46, 36, 78, nil, nil, nil, nil, nil, nil, -51, 3, 24, 60, 77, 79, -53, -47, -22, 6, nil, 33, 45, 72, nil, nil, nil, 80, -57, nil, nil, nil, -25, -10, 4, 13, 25, nil, nil, nil, 70, nil, nil, 81, -58, nil, -37, -23, nil, 1, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 0, 2},
		43, 81);

	constexpr int n{100000};
	{
		const std::vector<TreeNode> pool{make_bst(Shape::complete, n, 0)};
		test_big("n = 100000, complete tree, k = 1", pool, 1);
		test_big("n = 100000, complete tree, k = n", pool, n);
		test_big("n = 100000, complete tree, k = n / 2", pool, n / 2);
		test_big("n = 100000, complete tree, k = 33333", pool, 33333);
	}
	{
		const std::vector<TreeNode> pool{make_bst(Shape::left_chain, n, 0)};
		test_big("n = 100000, left chain, k = 1 (the deepest node)", pool, 1);
		test_big("n = 100000, left chain, k = n (the root)", pool, n);
		test_big("n = 100000, left chain, k = n / 2", pool, n / 2);
	}
	{
		const std::vector<TreeNode> pool{make_bst(Shape::right_chain, n, 0)};
		test_big("n = 100000, right chain, k = 1 (the root)", pool, 1);
		test_big("n = 100000, right chain, k = n (the deepest node)", pool, n);
		test_big("n = 100000, right chain, k = n / 2", pool, n / 2);
	}
	{
		const std::vector<TreeNode> pool{make_bst(Shape::zigzag, n, 0)};
		test_big("n = 100000, zigzag, k = 1", pool, 1);
		test_big("n = 100000, zigzag, k = n", pool, n);
		test_big("n = 100000, zigzag, k = n / 2", pool, n / 2);
	}
	{
		const std::vector<TreeNode> pool{make_bst(Shape::random, n, 1)};
		test_big("n = 100000, random tree, k = 1", pool, 1);
		test_big("n = 100000, random tree, k = n", pool, n);
		test_big("n = 100000, random tree, k = 77777", pool, 77777);
	}

	return 0;
}
```