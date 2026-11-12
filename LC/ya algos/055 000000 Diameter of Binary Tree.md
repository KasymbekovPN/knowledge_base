Задача 7 блока 7: Diameter of Binary Tree. Собираю тесты и проверяю.

Задача 7 блока 7 готова. Проверка по запуску: заглушка (возвращает `-1`) даёт 37 FAIL из 37; два разных эталона (рекурсия и итеративный обход с отображением высот) проходят все 37 без предупреждений и чисто под ASan/UBSan (рекурсивному под ASan нужен увеличенный стек, об этом ниже); три неверных варианта (только путь через корень, подсчёт в узлах вместо рёбер, высота дерева вместо диаметра) ловятся, а квадратичный вариант (пересчёт высоты для каждого узла) даёт `(too slow)` на цепочках.

## Diameter of Binary Tree

Дано бинарное дерево. Верните его диаметр: длину самого длинного пути между любыми двумя узлами, измеряемую в **рёбрах**. Путь может, но не обязан проходить через корень. Для пустого дерева и дерева из одного узла диаметр равен 0.

```cpp
int diameter(const TreeNode* root);
```

**Ограничения:** 0 ≤ n ≤ 10⁵, значения узлов роли не играют.

**Вопросы интервьюера:**

1. Почему нельзя просто взять сумму высот левого и правого поддеревьев корня? Приведите контрпример.
2. Как за один обход получить и высоту, и диаметр? Что возвращает рекурсия и что накапливается отдельно?
3. Какая сложность по времени и по памяти? В чём проблема наивного решения «высота для каждого узла»?
4. Как сделать обход без рекурсии? Что нужно, чтобы обработать узел раньше, чем его родителя?

**Цель:** O(n) по времени. **Ориентир по времени:** 12 минут.

```cpp
namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	int diameter([[maybe_unused]] const TreeNode* root) {
		return -1;
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

	void test_diameter(const std::string& label,
					   const std::vector<int>& level,
					   int expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			diameter(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	// a tree whose nodes are numbered 0..n-1, the node 0 is the root;
	// the parent of a node always has a smaller number than the node
	struct Tree {
		std::vector<TreeNode> pool;
		std::vector<std::size_t> parent;
	};

	// reference answer for large trees: the heights are computed bottom-up over the node numbers,
	// the longest path through a node is the sum of its two longest downward paths
	int expected_diameter(const Tree& tree) {
		const std::size_t count{tree.pool.size()};
		std::vector<int> best(count, 0);
		std::vector<int> second(count, 0);
		int result{0};
		for (std::size_t i{count}; i > 0; --i) {
			const std::size_t node{i - 1};
			result = std::max(result, best[node] + second[node]);
			if (node == 0) {
				break;
			}
			const std::size_t parent{tree.parent[node]};
			const int length{best[node] + 1};
			if (length > best[parent]) {
				second[parent] = best[parent];
				best[parent] = length;
			} else if (length > second[parent]) {
				second[parent] = length;
			}
		}
		return result;
	}

	void test_big(const std::string& label, const Tree& tree) {
		const int expected{expected_diameter(tree)};

		const auto start{std::chrono::steady_clock::now()};
		const int result{diameter(&tree.pool.front())};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random, two_chains, lopsided };

	Tree make_tree(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		Tree tree{std::vector<TreeNode>(count), std::vector<std::size_t>(count, 0)};
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
				case Shape::two_chains:
					// a left chain and a right chain of the same length hang off the root
					parent = i <= 2 ? 0 : i - 2;
					is_left = i % 2 == 1;
					break;
				case Shape::lopsided:
					// the root has a single leaf on the left and a complete tree on the right
					if (i == 1) {
						parent = 0;
						is_left = true;
					} else if (i == 2) {
						parent = 0;
						is_left = false;
					} else {
						parent = 2 + (i - 3) / 2;
						is_left = (i - 3) % 2 == 0;
					}
					break;
			}
			(is_left ? tree.pool[parent].left : tree.pool[parent].right) = &tree.pool[i];
			tree.parent[i] = parent;
		}
		return tree;
	}
}

int main() {

	test_diameter("description example 1",
		{1, 2, 3, 4, 5},
		3);
	test_diameter("description example 2, two nodes",
		{1, 2},
		1);
	test_diameter("empty tree",
		{},
		0);
	test_diameter("single node",
		{1},
		0);
	test_diameter("root with two leaves",
		{1, 2, 3},
		2);
	test_diameter("root with a right child only",
		{1, nil, 2},
		1);
	test_diameter("left chain of 5",
		{1, 2, nil, 3, nil, 4, nil, 5},
		4);
	test_diameter("right chain of 5",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		4);
	test_diameter("zigzag of 5",
		{1, 2, nil, nil, 3, 4, nil, nil, 5},
		4);
	test_diameter("complete tree of 7 nodes",
		{1, 2, 3, 4, 5, 6, 7},
		4);
	test_diameter("complete tree of 15 nodes",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		6);
	test_diameter("the longest path goes through the root, both arms are chains",
		{1, 2, 3, 4, nil, nil, 5, 6, nil, nil, 7},
		6);
	test_diameter("the longest path is inside the left subtree",
		{1, 2, 9, 3, 4, nil, nil, 5, nil, 7, nil, 6, nil, 8},
		6);
	test_diameter("the longest path is inside the right subtree",
		{1, 9, 2, nil, nil, 3, 4, 5, nil, 7, nil, 6, nil, 8},
		6);
	test_diameter("the root has only one child, the longest path is deeper",
		{1, nil, 2, 3, 6, 4, nil, 7, nil, 5, nil, 8},
		6);
	test_diameter("the deepest leaf is far from the root, the longest path is in the right subtree",
		{1, 2, 4, 3, nil, 5, 9, nil, nil, 6, nil, nil, 10, 7, nil, nil, 11, 8, nil, nil, 12},
		8);
	test_diameter("the longest path hangs off a node deep in the left subtree",
		{1, 2, 12, nil, 3, nil, nil, 4, 8, 5, nil, nil, 9, 6, nil, nil, 10, 7, nil, nil, 11},
		8);
	test_diameter("a long left chain and a single right leaf",
		{1, 2, 7, 3, nil, nil, nil, 4, nil, 5, nil, 6},
		6);
	test_diameter("the root has one child with two long chains",
		{1, 2, nil, 3, 6, 4, nil, nil, 7, 5, nil, nil, 8},
		6);
	test_diameter("equal values everywhere",
		{7, 7, 7, 7, 7, 7, 7},
		4);
	test_diameter("negative values",
		{-1, -2, -3, -4, -5},
		3);
	test_diameter("random 1, n = 3",
		{64, -77, nil, 99},
		2);
	test_diameter("random 2, n = 5",
		{75, -30, 45, 34, nil, nil, nil, 56},
		4);
	test_diameter("random 3, n = 8",
		{31, 54, -21, nil, nil, nil, -19, -72, nil, -96, nil, 97, nil, nil, -14},
		7);
	test_diameter("random 4, n = 12",
		{56, 26, nil, 85, 0, -43, -56, nil, 87, 83, -65, nil, nil, 50, nil, nil, nil, nil, -74, nil, 15},
		8);
	test_diameter("random 5, n = 20",
		{64, -71, 31, nil, -73, -98, -43, nil, -12, 74, -17, -64, -86, nil, 87, nil, nil, nil, 88, nil, nil, nil, nil, 43, 23, 37, nil, -20, nil, 45, 38, nil, -89},
		12);
	test_diameter("random 6, n = 30",
		{-43, 42, -52, nil, nil, -95, -34, 69, 96, -7, -84, -22, 99, 47, 31, nil, nil, -49, -71, 51, nil, nil, nil, -68, nil, nil, 28, nil, -16, 33, 55, nil, nil, nil, -27, nil, nil, nil, -5, nil, nil, nil, -52, nil, 89, nil, -14, 85, nil, 10, 58, nil, 74},
		14);
	test_diameter("random 7, n = 40",
		{4, -16, 27, -67, -84, -28, nil, -74, 44, 36, nil, 46, 91, 24, 46, -16, -32, 88, nil, 67, nil, -81, -37, 14, 91, nil, 37, nil, nil, nil, -97, 100, -60, -60, nil, nil, nil, -26, nil, 93, nil, 77, 29, nil, -13, -44, nil, nil, nil, nil, nil, nil, nil, nil, nil, 47, nil, -37, nil, nil, nil, nil, nil, nil, nil, -21, nil, nil, -68, nil, nil, -17, 94, nil, nil, -55, -54},
		15);
	test_diameter("random 8, n = 50",
		{-95, -88, 23, -99, 78, -89, nil, -87, 17, -63, 77, 53, -62, 5, -87, 64, -92, nil, 98, -50, -97, 26, nil, nil, nil, -84, 58, nil, nil, -87, 14, nil, -25, -68, -100, nil, 65, 9, -98, nil, nil, 39, -98, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 49, 63, nil, nil, nil, nil, nil, nil, -39, nil, -2, nil, -50, 13, 89, nil, 56, nil, nil, nil, nil, 31, 66, -11, nil, nil, nil, nil, nil, nil, 74, nil, -51, nil, nil, nil, -43, -93, nil, -82, 10, nil, nil, -85},
		18);

	test_big("n = 100000, complete tree", make_tree(Shape::complete, 100000, 0));
	test_big("n = 100000, left chain", make_tree(Shape::left_chain, 100000, 0));
	test_big("n = 100000, right chain", make_tree(Shape::right_chain, 100000, 0));
	test_big("n = 100000, zigzag", make_tree(Shape::zigzag, 100000, 0));
	test_big("n = 99999, a left chain and a right chain from the root", make_tree(Shape::two_chains, 99999, 0));
	test_big("n = 100000, a leaf and a complete tree under the root", make_tree(Shape::lopsided, 100000, 0));
	test_big("n = 100000, random tree", make_tree(Shape::random, 100000, 1));
	test_big("n = 100000, another random tree", make_tree(Shape::random, 100000, 2));

	return 0;
}
```

Два замечания к запуску. Проверка по времени (1 секунда) не умеет прерывать зависшую программу: квадратичное решение на цепочке из 10⁵ узлов будет считать очень долго, остановите его сами. Рекурсивное решение при глубине 10⁵ нормально работает без санитайзеров, но под `-fsanitize=address` может упасть по стеку (кадры ASan больше): это не ошибка алгоритма, увеличьте стек через `ulimit -s unlimited` или проверьте итеративный вариант.

Пришлите решение вложением. Когда будет готово, я его разберу и закончу списком «в журнал ошибок».

---

18:15 - 18:38

---

```

      1
     / \
    2   3
   / \
  4   5

```


```
        1
       / \
      2   9
     / \
    3   4
   /     \
  5       7
 /         \
6           8
```

---

```cpp
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <queue>
#include <tuple>
#include <cstddef>
#include <utility>
#include <random>
#include <algorithm>
#include <chrono>

namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	std::pair<int, int> find(const TreeNode* node, int& diameter) {
		const TreeNode* left{node->left};
		const TreeNode* right{node->right};
		
		std::pair<int, int> result{0, 0};
		if (left) {
			const auto lresult{find(left, diameter)};
			result.first = std::max(lresult.first, lresult.second) + 1;
		}
		if (right) {
			const auto rresult{find(right, diameter)}
			result.second = std::max(rresult.first, rresult.second) + 1;
		}
		
		const auto sum{result.first + result.second};
		if (diameter < sum) { diameter = sum; }
	
		return result;
	}

	int diameter(const TreeNode* root) {
		int diameter{0};
		const auto _ = find(root, diameter);
		(void)_;
		return diameter;
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

	void test_diameter(const std::string& label,
					   const std::vector<int>& level,
					   int expected) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			diameter(root) == expected ? "OK" : "FAIL") << std::flush;
	}

	// a tree whose nodes are numbered 0..n-1, the node 0 is the root;
	// the parent of a node always has a smaller number than the node
	struct Tree {
		std::vector<TreeNode> pool;
		std::vector<std::size_t> parent;
	};

	// reference answer for large trees: the heights are computed bottom-up over the node numbers,
	// the longest path through a node is the sum of its two longest downward paths
	int expected_diameter(const Tree& tree) {
		const std::size_t count{tree.pool.size()};
		std::vector<int> best(count, 0);
		std::vector<int> second(count, 0);
		int result{0};
		for (std::size_t i{count}; i > 0; --i) {
			const std::size_t node{i - 1};
			result = std::max(result, best[node] + second[node]);
			if (node == 0) {
				break;
			}
			const std::size_t parent{tree.parent[node]};
			const int length{best[node] + 1};
			if (length > best[parent]) {
				second[parent] = best[parent];
				best[parent] = length;
			} else if (length > second[parent]) {
				second[parent] = length;
			}
		}
		return result;
	}

	void test_big(const std::string& label, const Tree& tree) {
		const int expected{expected_diameter(tree)};

		const auto start{std::chrono::steady_clock::now()};
		const int result{diameter(&tree.pool.front())};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			result == expected ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random, two_chains, lopsided };

	Tree make_tree(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		Tree tree{std::vector<TreeNode>(count), std::vector<std::size_t>(count, 0)};
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
				case Shape::two_chains:
					// a left chain and a right chain of the same length hang off the root
					parent = i <= 2 ? 0 : i - 2;
					is_left = i % 2 == 1;
					break;
				case Shape::lopsided:
					// the root has a single leaf on the left and a complete tree on the right
					if (i == 1) {
						parent = 0;
						is_left = true;
					} else if (i == 2) {
						parent = 0;
						is_left = false;
					} else {
						parent = 2 + (i - 3) / 2;
						is_left = (i - 3) % 2 == 0;
					}
					break;
			}
			(is_left ? tree.pool[parent].left : tree.pool[parent].right) = &tree.pool[i];
			tree.parent[i] = parent;
		}
		return tree;
	}
}

int main() {

	test_diameter("description example 1",
		{1, 2, 3, 4, 5},
		3);
	test_diameter("description example 2, two nodes",
		{1, 2},
		1);
	test_diameter("empty tree",
		{},
		0);
	test_diameter("single node",
		{1},
		0);
	test_diameter("root with two leaves",
		{1, 2, 3},
		2);
	test_diameter("root with a right child only",
		{1, nil, 2},
		1);
	test_diameter("left chain of 5",
		{1, 2, nil, 3, nil, 4, nil, 5},
		4);
	test_diameter("right chain of 5",
		{1, nil, 2, nil, 3, nil, 4, nil, 5},
		4);
	test_diameter("zigzag of 5",
		{1, 2, nil, nil, 3, 4, nil, nil, 5},
		4);
	test_diameter("complete tree of 7 nodes",
		{1, 2, 3, 4, 5, 6, 7},
		4);
	test_diameter("complete tree of 15 nodes",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		6);
	test_diameter("the longest path goes through the root, both arms are chains",
		{1, 2, 3, 4, nil, nil, 5, 6, nil, nil, 7},
		6);
	test_diameter("the longest path is inside the left subtree",
		{1, 2, 9, 3, 4, nil, nil, 5, nil, 7, nil, 6, nil, 8},
		6);
	test_diameter("the longest path is inside the right subtree",
		{1, 9, 2, nil, nil, 3, 4, 5, nil, 7, nil, 6, nil, 8},
		6);
	test_diameter("the root has only one child, the longest path is deeper",
		{1, nil, 2, 3, 6, 4, nil, 7, nil, 5, nil, 8},
		6);
	test_diameter("the deepest leaf is far from the root, the longest path is in the right subtree",
		{1, 2, 4, 3, nil, 5, 9, nil, nil, 6, nil, nil, 10, 7, nil, nil, 11, 8, nil, nil, 12},
		8);
	test_diameter("the longest path hangs off a node deep in the left subtree",
		{1, 2, 12, nil, 3, nil, nil, 4, 8, 5, nil, nil, 9, 6, nil, nil, 10, 7, nil, nil, 11},
		8);
	test_diameter("a long left chain and a single right leaf",
		{1, 2, 7, 3, nil, nil, nil, 4, nil, 5, nil, 6},
		6);
	test_diameter("the root has one child with two long chains",
		{1, 2, nil, 3, 6, 4, nil, nil, 7, 5, nil, nil, 8},
		6);
	test_diameter("equal values everywhere",
		{7, 7, 7, 7, 7, 7, 7},
		4);
	test_diameter("negative values",
		{-1, -2, -3, -4, -5},
		3);
	test_diameter("random 1, n = 3",
		{64, -77, nil, 99},
		2);
	test_diameter("random 2, n = 5",
		{75, -30, 45, 34, nil, nil, nil, 56},
		4);
	test_diameter("random 3, n = 8",
		{31, 54, -21, nil, nil, nil, -19, -72, nil, -96, nil, 97, nil, nil, -14},
		7);
	test_diameter("random 4, n = 12",
		{56, 26, nil, 85, 0, -43, -56, nil, 87, 83, -65, nil, nil, 50, nil, nil, nil, nil, -74, nil, 15},
		8);
	test_diameter("random 5, n = 20",
		{64, -71, 31, nil, -73, -98, -43, nil, -12, 74, -17, -64, -86, nil, 87, nil, nil, nil, 88, nil, nil, nil, nil, 43, 23, 37, nil, -20, nil, 45, 38, nil, -89},
		12);
	test_diameter("random 6, n = 30",
		{-43, 42, -52, nil, nil, -95, -34, 69, 96, -7, -84, -22, 99, 47, 31, nil, nil, -49, -71, 51, nil, nil, nil, -68, nil, nil, 28, nil, -16, 33, 55, nil, nil, nil, -27, nil, nil, nil, -5, nil, nil, nil, -52, nil, 89, nil, -14, 85, nil, 10, 58, nil, 74},
		14);
	test_diameter("random 7, n = 40",
		{4, -16, 27, -67, -84, -28, nil, -74, 44, 36, nil, 46, 91, 24, 46, -16, -32, 88, nil, 67, nil, -81, -37, 14, 91, nil, 37, nil, nil, nil, -97, 100, -60, -60, nil, nil, nil, -26, nil, 93, nil, 77, 29, nil, -13, -44, nil, nil, nil, nil, nil, nil, nil, nil, nil, 47, nil, -37, nil, nil, nil, nil, nil, nil, nil, -21, nil, nil, -68, nil, nil, -17, 94, nil, nil, -55, -54},
		15);
	test_diameter("random 8, n = 50",
		{-95, -88, 23, -99, 78, -89, nil, -87, 17, -63, 77, 53, -62, 5, -87, 64, -92, nil, 98, -50, -97, 26, nil, nil, nil, -84, 58, nil, nil, -87, 14, nil, -25, -68, -100, nil, 65, 9, -98, nil, nil, 39, -98, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 49, 63, nil, nil, nil, nil, nil, nil, -39, nil, -2, nil, -50, 13, 89, nil, 56, nil, nil, nil, nil, 31, 66, -11, nil, nil, nil, nil, nil, nil, 74, nil, -51, nil, nil, nil, -43, -93, nil, -82, 10, nil, nil, -85},
		18);

	test_big("n = 100000, complete tree", make_tree(Shape::complete, 100000, 0));
	test_big("n = 100000, left chain", make_tree(Shape::left_chain, 100000, 0));
	test_big("n = 100000, right chain", make_tree(Shape::right_chain, 100000, 0));
	test_big("n = 100000, zigzag", make_tree(Shape::zigzag, 100000, 0));
	test_big("n = 99999, a left chain and a right chain from the root", make_tree(Shape::two_chains, 99999, 0));
	test_big("n = 100000, a leaf and a complete tree under the root", make_tree(Shape::lopsided, 100000, 0));
	test_big("n = 100000, random tree", make_tree(Shape::random, 100000, 1));
	test_big("n = 100000, another random tree", make_tree(Shape::random, 100000, 2));

	return 0;
}
```