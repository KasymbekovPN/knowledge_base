Задача 10 блока 7: Serialize and Deserialize Binary Tree. Собираю тесты и проверяю.

Задача 10 блока 7 готова. Проверка по запуску: заглушка собирается без предупреждений и даёт 52 FAIL из 53; два эталона разного формата (рекурсия по префиксному обходу и обход по уровням) проходят все 53 и чисто под ASan/UBSan; пять неверных вариантов (без пометок пустых детей, без разделителей, потеря знака минус, 16-битные значения, склейка строки `s = s + ...`) ловятся или помечаются `(too slow)`. Ещё один медленный вариант (рекурсивный `substr` при разборе) на первой же большой цепочке не завершается за разумное время, поэтому ниже есть предупреждение остановить программу вручную.

## Serialize and Deserialize Binary Tree

Напишите две функции: `serialize` превращает бинарное дерево в строку, `deserialize` по этой строке (и только по ней) строит такое же дерево. Формат вы выбираете сами. Условия: деревья с одинаковой формой и значениями дают одну и ту же строку, разные деревья дают разные строки, а длина строки в среднем не больше 16 байт на узел.

Узлы результата `deserialize` создаются в переданном `pool`, чей запас (`capacity`) равен `data.size() + 1`: добавляйте узлы через `pool.push_back(...)` и не выходите за этот запас (формат должен тратить на каждый узел хотя бы один байт), иначе вектор переедет, и указатели станут недействительными.

```cpp
std::string serialize(const TreeNode* root);
TreeNode* deserialize(const std::string& data, std::vector<TreeNode>& pool);
```

**Ограничения:** 0 ≤ n ≤ 10⁵, значения лежат во всём диапазоне `int`.

**Вопросы интервьюера:**

1. Какой формат вы выбрали и почему он однозначен? Почему нельзя записать только значения прямого обхода? Приведите два разных дерева с одинаковыми значениями в прямом обходе.
2. Как вы отделяете числа друг от друга и обозначаете отсутствующих детей? Что произойдёт с отрицательными числами и с числами из нескольких цифр?
3. Какая сложность по времени и по памяти у каждой функции? Что делать, если вы собираете строку в цикле?
4. Как обойтись без рекурсии (на цепочке из 10⁵ узлов) и какой формат при этом удобнее (по уровням или в глубину)?

**Цель:** O(n) по времени и памяти на каждую функцию. **Ориентир по времени:** 20 минут.

```cpp
namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};

	// any text format you like, but it has to take at most 16 bytes per node on average
	std::string serialize([[maybe_unused]] const TreeNode* root) {
		return "";
	}

	// the nodes of the result must be created in `pool`, whose capacity is data.size() + 1,
	// so the pool must never be reallocated
	TreeNode* deserialize([[maybe_unused]] const std::string& data,
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

	// serialize, then deserialize from the text only; the text must not be too long
	bool round_trip(const TreeNode* root, std::size_t count) {
		const std::string data{serialize(root)};
		std::vector<TreeNode> pool;
		pool.reserve(data.size() + 1);
		const TreeNode* result{deserialize(data, pool)};
		return same_tree(result, root) && data.size() <= 16 * count + 16;
	}

	void test_round_trip(const std::string& label, const std::vector<int>& level) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			round_trip(root, pool.size()) ? "OK" : "FAIL") << std::flush;
	}

	// the same, but the values are given separately, in the order of the nodes in `level`
	void test_round_trip_values(const std::string& label,
								const std::vector<int>& level,
								const std::vector<int>& values) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		for (std::size_t i{0}; i < pool.size(); ++i) {
			pool[i].val = values[i];
		}
		std::cout << std::format("{}: {}\n",
			label,
			round_trip(root, pool.size()) ? "OK" : "FAIL") << std::flush;
	}

	// two different trees must give two different texts, and the same tree always the same text
	void test_different(const std::string& label,
						const std::vector<int>& first,
						const std::vector<int>& second) {
		std::vector<TreeNode> pool_a;
		std::vector<TreeNode> pool_b;
		std::vector<TreeNode> pool_c;
		const std::string a{serialize(build_tree(first, pool_a))};
		const std::string b{serialize(build_tree(second, pool_b))};
		const std::string c{serialize(build_tree(first, pool_c))};
		std::cout << std::format("{}: {}\n",
			label,
			a != b && a == c ? "OK" : "FAIL") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// a tree of the given shape, the node 0 is the root; the values are random over the whole int range
	std::vector<TreeNode> make_tree(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		std::vector<TreeNode> pool(count);
		std::mt19937 gen{seed};
		for (std::size_t i{0}; i < count; ++i) {
			pool[i].val = static_cast<int>(gen());
		}
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

		const auto start{std::chrono::steady_clock::now()};
		const bool ok{round_trip(&original.front(), original.size())};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			ok ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}
}

int main() {

	test_round_trip("description example 1",
		{1, 2, 3, nil, nil, 4, 5});
	test_round_trip("empty tree",
		{});
	test_round_trip("single node",
		{5});
	test_round_trip("two nodes, the left child",
		{1, 2});
	test_round_trip("two nodes, the right child",
		{1, nil, 2});
	test_round_trip("root with two leaves",
		{1, 2, 3});
	test_round_trip("left chain of 5",
		{1, 2, nil, 3, nil, 4, nil, 5});
	test_round_trip("right chain of 5",
		{1, nil, 2, nil, 3, nil, 4, nil, 5});
	test_round_trip("zigzag of 5",
		{1, 2, nil, nil, 3, 4, nil, nil, 5});
	test_round_trip("complete tree of 7 nodes",
		{1, 2, 3, 4, 5, 6, 7});
	test_round_trip("complete tree of 15 nodes",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
	test_round_trip("only the left child has children",
		{1, 2, 3, 4, 5, nil, nil, 6, 7});
	test_round_trip("only the right child has children",
		{1, 2, 3, nil, nil, 4, 5, nil, nil, 6, 7});
	test_round_trip("the left subtree is much deeper",
		{1, 2, 3, 4, nil, nil, nil, 5, nil, 6});
	test_round_trip("the right subtree is much deeper",
		{1, 2, 3, nil, nil, nil, 4, nil, 5, nil, 6});
	test_round_trip("equal values everywhere",
		{7, 7, 7, 7, 7, 7, 7});
	test_round_trip("zero values",
		{0, 0, 0, 0, 0, 0, 0});
	test_round_trip("negative values",
		{-1, -2, -3, -4, -5, -6, -7});
	test_round_trip("multi-digit values",
		{100, -100, 99, 10, -11, 12, -13});
	test_round_trip("the values are not sorted",
		{9, 4, 7, 8, 1, 2, 6});
	test_round_trip("a node with a single right child inside the left subtree",
		{1, 2, 3, nil, 4, 5, nil, nil, nil, 6});
	test_round_trip("a node with a single left child inside the right subtree",
		{1, 2, 3, 4, nil, nil, 5, nil, nil, 6});
	test_round_trip("random 1, n = 3",
		{34, 91, -6});
	test_round_trip("random 2, n = 5",
		{84, -18, -72, nil, nil, nil, -76, -58});
	test_round_trip("random 3, n = 8",
		{-26, 1, 16, nil, -26, nil, 98, nil, nil, 81, 26, nil, -16});
	test_round_trip("random 4, n = 12",
		{9, nil, 37, -43, 69, nil, 26, 63, 39, -20, nil, 89, nil, 56, nil, nil, -95, 6});
	test_round_trip("random 5, n = 20",
		{48, 77, -30, -27, -59, 60, 6, 30, nil, -86, nil, nil, 84, 59, 88, nil, -52, -71, nil, 51, nil, nil, 69, -100, nil, nil, 25, nil, nil, nil, nil, nil, nil, nil, -24, nil, nil, nil, 21});
	test_round_trip("random 6, n = 30",
		{83, -7, -18, 71, -25, -47, -39, -36, -98, nil, 62, -90, -68, -91, -52, nil, -29, nil, nil, nil, nil, 44, nil, -24, nil, 37, nil, nil, nil, -98, -36, nil, nil, nil, -62, 8, 59, nil, nil, nil, nil, nil, 37, -99, 98, nil, -30, nil, nil, nil, nil, 41, nil, 34, nil, -89});
	test_round_trip("random 7, n = 40",
		{-97, -59, -34, 71, -40, 58, 82, 61, 50, 95, 15, 61, 3, nil, nil, 62, 14, nil, 47, -95, 100, -21, -21, nil, nil, nil, nil, -49, nil, nil, 69, nil, 25, nil, -92, nil, -72, nil, nil, nil, nil, 26, -67, 43, -27, nil, 100, 7, nil, nil, 96, nil, 39, nil, nil, nil, 16, nil, -66, nil, nil, nil, nil, nil, nil, 56, -71, nil, -73, nil, nil, nil, nil, nil, nil, 51, nil, nil, 65});
	test_round_trip("random 8, n = 50",
		{78, 49, 60, 25, 86, -65, 42, nil, -20, 58, -24, -3, -39, 82, 47, nil, nil, -8, nil, nil, nil, 91, nil, -72, -28, nil, nil, 69, -95, -84, -45, nil, 50, nil, nil, -98, 33, nil, -83, nil, 31, -91, 59, -74, nil, -43, nil, 92, -38, nil, nil, nil, 52, 83, 48, -18, 82, -85, nil, nil, 87, nil, nil, nil, 64, nil, nil, nil, nil, 75, 81, 8, nil, nil, -32, nil, nil, nil, nil, 19, nil, nil, nil, -73, nil, nil, 74, nil, 28, nil, nil, nil, 79});

	test_different("the left child against the right child",
		{1, 2},
		{1, nil, 2});
	test_different("the same preorder, the second node is a left child or a right child of the root",
		{1, 2, 3},
		{1, 2, nil, nil, 3});
	test_different("the same preorder, a left child against a right chain",
		{10, 5, 15},
		{10, nil, 5, nil, 15});
	test_different("a left chain against a right chain",
		{1, 2, nil, 3},
		{1, nil, 2, nil, 3});
	test_different("the same values, swapped children",
		{1, 2, 3},
		{1, 3, 2});
	test_different("a single node 12 against two nodes 1 and 2",
		{12},
		{1, 2});
	test_different("two nodes 1 and 12 against two nodes 11 and 2",
		{1, 12},
		{11, 2});
	test_different("a tree against the same tree with one more leaf",
		{1, 2, 3},
		{1, 2, 3, 4});
	test_different("an empty tree against a single node",
		{},
		{0});
	test_different("the value 0 against a missing child",
		{1, 0},
		{1});
	test_different("negative value against a positive one",
		{-1, 2},
		{1, 2});

	constexpr int lowest{std::numeric_limits<int>::min()};
	constexpr int highest{std::numeric_limits<int>::max()};
	test_round_trip_values("the lowest int alone", {1}, {lowest});
	test_round_trip_values("the highest int alone", {1}, {highest});
	test_round_trip_values("lowest, 0, highest", {1, 1, 1}, {0, lowest, highest});
	test_round_trip_values("a chain from the highest down to the lowest", {1, 1, nil, 1, nil, 1}, {highest, 1, 0, lowest});
	test_round_trip_values("values with different numbers of digits", {1, 1, 1, 1, 1, 1, 1}, {7, -70, 700, -7000, 70000, -700000, 7000000});
	test_round_trip_values("a value that looks like the missing child marker", {1, 1, 1}, {1000, -1000, 1000});

	test_big("n = 100000, complete tree", Shape::complete, 100000, 0);
	test_big("n = 100000, left chain", Shape::left_chain, 100000, 0);
	test_big("n = 100000, right chain", Shape::right_chain, 100000, 0);
	test_big("n = 100000, zigzag", Shape::zigzag, 100000, 0);
	test_big("n = 100000, random tree", Shape::random, 100000, 1);
	test_big("n = 100000, another random tree", Shape::random, 100000, 2);

	return 0;
}
```

Три замечания к запуску:

- Проверка по времени (1 секунда) не умеет прерывать зависшую программу: если разбор строки сделан через копирование остатка (`substr` на каждый узел), на цепочке из 10⁵ узлов он будет считать очень долго, остановите его сами.
- Рекурсивный разбор на цепочке из 10⁵ узлов с обычным стеком (8 МБ) у меня упал по переполнению стека при глубине 10⁵, а итеративный вариант (по уровням, очередь) прошёл на обычном стеке. Рекурсивный пройдёт с `ulimit -s unlimited`. Это не ошибка алгоритма, но сам вопрос 4 о том, как обойтись без рекурсии.
- Под `-fsanitize=address` рекурсия тоже требует увеличенного стека (`ulimit -s unlimited`).

Пришлите решение вложением. Когда будет готово, я его разберу и закончу списком «в журнал ошибок».

---



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

---

```cpp
#include <iostream>
#include <format>
#include <cstddef>
#include <utility>
#include <vector>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <string>
#include <chrono>
#include <random>
#include <cstring>
#include <limits>

namespace {
	struct TreeNode {
		int val{0};
		TreeNode* left{nullptr};
		TreeNode* right{nullptr};
	};

	constexpr int nil{1000};
	constexpr std::size_t SIZE{sizeof(4)};

	std::string serialize_value(const int value) {
		std::string buffer;
		buffer.resize(SIZE);
		
		std::memcpy(&buffer[0], &value, SIZE);
	
		return buffer;
	}
	
	int deserilize_value(const std::string& buffer) {
		int value;
		std::memcpy(&value, &buffer[0], SIZE);
	
		return value;
	}

	void fill_preorder(const TreeNode* node, std::string& data) {
		if (node == nullptr) return;
		data += serialize_value(node->val);
		fill_preorder(node->left, data);
		fill_preorder(node->right, data);
	}
	 
	void fill_inorder(const TreeNode* node, std::string& data) {
		if (node == nullptr) return;
		fill_inorder(node->left, data);
		data += serialize_value(node->val);
		fill_inorder(node->right, data);
	}
	
	TreeNode* build(const std::vector<int>& preorder,
                    const std::unordered_map<int, std::size_t>& positions,
	                std::size_t& next,
	                std::size_t lo,
	                std::size_t hi,
	                std::vector<TreeNode>& pool) {
                
        if (lo >= hi) return nullptr;
        pool.push_back(TreeNode{preorder[next], nullptr, nullptr});
        TreeNode* node{&pool.back()};
        
        const std::size_t mid{positions.at(preorder[next])};
	    ++next;
	    
	    node->left = build(preorder, positions, next, lo, mid, pool);
	    node->right = build(preorder, positions, next, mid + 1, hi, pool);
	    
	    return node;
	}

	// any text format you like, but it has to take at most 16 bytes per node on average
	std::string serialize(const TreeNode* root) {
		std::string result;
		fill_preorder(root, result);
		fill_inorder(root, result);

		return result;
	}

	// the nodes of the result must be created in `pool`, whose capacity is data.size() + 1,
	// so the pool must never be reallocated
	TreeNode* deserialize(const std::string& data,
						  std::vector<TreeNode>& pool) {
	
		const std::size_t half_size{data.size() / 2};
		const std::size_t size{half_size / SIZE};
	
		std::vector<int> preorder;
		preorder.reserve(size);
		std::unordered_map<int, std::size_t> positions;
		
		for (std::size_t i{0}; i < size; ++i) {
			preorder.push_back(deserilize_value(data.substr(SIZE * i, SIZE)));
			positions.emplace(
				deserilize_value(data.substr(half_size + SIZE * i, SIZE)),
				i
			);
		}

		std::size_t next{0};
		return build(preorder, positions, next, 0, size, pool);
	}
	
	//-----------------------------------------------
	
	void write(const TreeNode* node, std::string& data) {
		if (node == nullptr) {
			data.push_back('\0');
			return;
		}
		
		data.push_back('\1');
		data.append(reinterpret_cast<const char*>(*node->val), sizeof node->val);
		write(node->left, data);
		write(node->right, data);
	}
}

	TreeNode* read(const std::string& data, std::size_t& position, std::vector<TreeNode>& pool) {
		if (data[position++] == '\0') return nullptr;
		
		int value{0};
		std::memcpy(&value, data.data() + position, sizeof value);
		
		position += sizeof value;
		pool.push_back(TreeNode{value, nullptr, nullptr});
		TreeNode* node{&pool.back()};
		node->left = read(data, position, pool);
		node->right = read(data, position, pool);
		
		return node;
	}
}

	std::string serialize(const TreeNode* root) {
		return write(root);
	}
	
	TreeNode* deserialize(const std::string& data,
						  std::vector<TreeNode>& pool) {
		std::size_t position{0};
		return read(data, position, pool);
	}
	
	//-----------------------------------------------

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

	// serialize, then deserialize from the text only; the text must not be too long
	bool round_trip(const TreeNode* root, std::size_t count) {
		const std::string data{serialize(root)};
		std::vector<TreeNode> pool;
		pool.reserve(data.size() + 1);
		const TreeNode* result{deserialize(data, pool)};
		return same_tree(result, root) && data.size() <= 16 * count + 16;
	}

	void test_round_trip(const std::string& label, const std::vector<int>& level) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		std::cout << std::format("{}: {}\n",
			label,
			round_trip(root, pool.size()) ? "OK" : "FAIL") << std::flush;
	}

	// the same, but the values are given separately, in the order of the nodes in `level`
	void test_round_trip_values(const std::string& label,
								const std::vector<int>& level,
								const std::vector<int>& values) {
		std::vector<TreeNode> pool;
		const TreeNode* root{build_tree(level, pool)};
		for (std::size_t i{0}; i < pool.size(); ++i) {
			pool[i].val = values[i];
		}
		std::cout << std::format("{}: {}\n",
			label,
			round_trip(root, pool.size()) ? "OK" : "FAIL") << std::flush;
	}

	// two different trees must give two different texts, and the same tree always the same text
	void test_different(const std::string& label,
						const std::vector<int>& first,
						const std::vector<int>& second) {
		std::vector<TreeNode> pool_a;
		std::vector<TreeNode> pool_b;
		std::vector<TreeNode> pool_c;
		const std::string a{serialize(build_tree(first, pool_a))};
		const std::string b{serialize(build_tree(second, pool_b))};
		const std::string c{serialize(build_tree(first, pool_c))};
		std::cout << std::format("{}: {}\n",
			label,
			a != b && a == c ? "OK" : "FAIL") << std::flush;
	}

	enum class Shape { complete, left_chain, right_chain, zigzag, random };

	// a tree of the given shape, the node 0 is the root; the values are random over the whole int range
	std::vector<TreeNode> make_tree(Shape shape, int n, unsigned seed) {
		const std::size_t count{static_cast<std::size_t>(n)};
		std::vector<TreeNode> pool(count);
		std::mt19937 gen{seed};
		for (std::size_t i{0}; i < count; ++i) {
			pool[i].val = static_cast<int>(gen());
		}
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

		const auto start{std::chrono::steady_clock::now()};
		const bool ok{round_trip(&original.front(), original.size())};
		const auto elapsed{std::chrono::steady_clock::now() - start};

		const bool in_time{elapsed < std::chrono::seconds(1)};
		std::cout << std::format("{}: {}{}\n",
			label,
			ok ? "OK" : "FAIL",
			in_time ? "" : " (too slow)") << std::flush;
	}
}

int main() {

	test_round_trip("description example 1",
		{1, 2, 3, nil, nil, 4, 5});
	test_round_trip("empty tree",
		{});
	test_round_trip("single node",
		{5});
	test_round_trip("two nodes, the left child",
		{1, 2});
	test_round_trip("two nodes, the right child",
		{1, nil, 2});
	test_round_trip("root with two leaves",
		{1, 2, 3});
	test_round_trip("left chain of 5",
		{1, 2, nil, 3, nil, 4, nil, 5});
	test_round_trip("right chain of 5",
		{1, nil, 2, nil, 3, nil, 4, nil, 5});
	test_round_trip("zigzag of 5",
		{1, 2, nil, nil, 3, 4, nil, nil, 5});
	test_round_trip("complete tree of 7 nodes",
		{1, 2, 3, 4, 5, 6, 7});
	test_round_trip("complete tree of 15 nodes",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
	test_round_trip("only the left child has children",
		{1, 2, 3, 4, 5, nil, nil, 6, 7});
	test_round_trip("only the right child has children",
		{1, 2, 3, nil, nil, 4, 5, nil, nil, 6, 7});
	test_round_trip("the left subtree is much deeper",
		{1, 2, 3, 4, nil, nil, nil, 5, nil, 6});
	test_round_trip("the right subtree is much deeper",
		{1, 2, 3, nil, nil, nil, 4, nil, 5, nil, 6});
	test_round_trip("equal values everywhere",
		{7, 7, 7, 7, 7, 7, 7});
	test_round_trip("zero values",
		{0, 0, 0, 0, 0, 0, 0});
	test_round_trip("negative values",
		{-1, -2, -3, -4, -5, -6, -7});
	test_round_trip("multi-digit values",
		{100, -100, 99, 10, -11, 12, -13});
	test_round_trip("the values are not sorted",
		{9, 4, 7, 8, 1, 2, 6});
	test_round_trip("a node with a single right child inside the left subtree",
		{1, 2, 3, nil, 4, 5, nil, nil, nil, 6});
	test_round_trip("a node with a single left child inside the right subtree",
		{1, 2, 3, 4, nil, nil, 5, nil, nil, 6});
	test_round_trip("random 1, n = 3",
		{34, 91, -6});
	test_round_trip("random 2, n = 5",
		{84, -18, -72, nil, nil, nil, -76, -58});
	test_round_trip("random 3, n = 8",
		{-26, 1, 16, nil, -26, nil, 98, nil, nil, 81, 26, nil, -16});
	test_round_trip("random 4, n = 12",
		{9, nil, 37, -43, 69, nil, 26, 63, 39, -20, nil, 89, nil, 56, nil, nil, -95, 6});
	test_round_trip("random 5, n = 20",
		{48, 77, -30, -27, -59, 60, 6, 30, nil, -86, nil, nil, 84, 59, 88, nil, -52, -71, nil, 51, nil, nil, 69, -100, nil, nil, 25, nil, nil, nil, nil, nil, nil, nil, -24, nil, nil, nil, 21});
	test_round_trip("random 6, n = 30",
		{83, -7, -18, 71, -25, -47, -39, -36, -98, nil, 62, -90, -68, -91, -52, nil, -29, nil, nil, nil, nil, 44, nil, -24, nil, 37, nil, nil, nil, -98, -36, nil, nil, nil, -62, 8, 59, nil, nil, nil, nil, nil, 37, -99, 98, nil, -30, nil, nil, nil, nil, 41, nil, 34, nil, -89});
	test_round_trip("random 7, n = 40",
		{-97, -59, -34, 71, -40, 58, 82, 61, 50, 95, 15, 61, 3, nil, nil, 62, 14, nil, 47, -95, 100, -21, -21, nil, nil, nil, nil, -49, nil, nil, 69, nil, 25, nil, -92, nil, -72, nil, nil, nil, nil, 26, -67, 43, -27, nil, 100, 7, nil, nil, 96, nil, 39, nil, nil, nil, 16, nil, -66, nil, nil, nil, nil, nil, nil, 56, -71, nil, -73, nil, nil, nil, nil, nil, nil, 51, nil, nil, 65});
	test_round_trip("random 8, n = 50",
		{78, 49, 60, 25, 86, -65, 42, nil, -20, 58, -24, -3, -39, 82, 47, nil, nil, -8, nil, nil, nil, 91, nil, -72, -28, nil, nil, 69, -95, -84, -45, nil, 50, nil, nil, -98, 33, nil, -83, nil, 31, -91, 59, -74, nil, -43, nil, 92, -38, nil, nil, nil, 52, 83, 48, -18, 82, -85, nil, nil, 87, nil, nil, nil, 64, nil, nil, nil, nil, 75, 81, 8, nil, nil, -32, nil, nil, nil, nil, 19, nil, nil, nil, -73, nil, nil, 74, nil, 28, nil, nil, nil, 79});

	test_different("the left child against the right child",
		{1, 2},
		{1, nil, 2});
	test_different("the same preorder, the second node is a left child or a right child of the root",
		{1, 2, 3},
		{1, 2, nil, nil, 3});
	test_different("the same preorder, a left child against a right chain",
		{10, 5, 15},
		{10, nil, 5, nil, 15});
	test_different("a left chain against a right chain",
		{1, 2, nil, 3},
		{1, nil, 2, nil, 3});
	test_different("the same values, swapped children",
		{1, 2, 3},
		{1, 3, 2});
	test_different("a single node 12 against two nodes 1 and 2",
		{12},
		{1, 2});
	test_different("two nodes 1 and 12 against two nodes 11 and 2",
		{1, 12},
		{11, 2});
	test_different("a tree against the same tree with one more leaf",
		{1, 2, 3},
		{1, 2, 3, 4});
	test_different("an empty tree against a single node",
		{},
		{0});
	test_different("the value 0 against a missing child",
		{1, 0},
		{1});
	test_different("negative value against a positive one",
		{-1, 2},
		{1, 2});

	constexpr int lowest{std::numeric_limits<int>::min()};
	constexpr int highest{std::numeric_limits<int>::max()};
	test_round_trip_values("the lowest int alone", {1}, {lowest});
	test_round_trip_values("the highest int alone", {1}, {highest});
	test_round_trip_values("lowest, 0, highest", {1, 1, 1}, {0, lowest, highest});
	test_round_trip_values("a chain from the highest down to the lowest", {1, 1, nil, 1, nil, 1}, {highest, 1, 0, lowest});
	test_round_trip_values("values with different numbers of digits", {1, 1, 1, 1, 1, 1, 1}, {7, -70, 700, -7000, 70000, -700000, 7000000});
	test_round_trip_values("a value that looks like the missing child marker", {1, 1, 1}, {1000, -1000, 1000});

	test_big("n = 100000, complete tree", Shape::complete, 100000, 0);
	test_big("n = 100000, left chain", Shape::left_chain, 100000, 0);
	test_big("n = 100000, right chain", Shape::right_chain, 100000, 0);
	test_big("n = 100000, zigzag", Shape::zigzag, 100000, 0);
	test_big("n = 100000, random tree", Shape::random, 100000, 1);
	test_big("n = 100000, another random tree", Shape::random, 100000, 2);

	return 0;
}
```