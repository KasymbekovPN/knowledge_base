## 347. Top K Frequent Elements (Medium)

Дан целочисленный массив `nums` и число `k`. Нужно вернуть `k` **самых частых** элементов. Порядок в ответе произвольный.

Гарантируется, что ответ единственный: либо `k` равно числу различных значений, либо частота `k`-го по частоте элемента строго больше частоты `(k+1)`-го.

**Примеры**

- `nums = [1,1,1,2,2,3]`, `k = 2` → `[1,2]`
- `nums = [1]`, `k = 1` → `[1]`

**Сигнатура**

```cpp
std::vector<int> top_k_frequent(const std::vector<int>& nums, int k);
```

**Ограничения**

- `1 <= nums.length <= 10^5`
- `-10^4 <= nums[i] <= 10^4`
- `k` лежит в диапазоне `[1, число различных значений]`
- Ответ единственный.

**Вопросы интервьюера**

1. Какое решение через подсчёт частот и сортировку по частоте? Какая у него сложность?
2. Можно ли лучше, чем O(n log n)? Какие есть варианты: куча на `k` элементов, `nth_element`, корзины по частоте?
3. Какой размер у кучи и какой она должна быть: min-heap или max-heap? Почему?
4. Как работает сортировка корзинами (bucket sort) и почему число корзин равно `n + 1`?
5. Что выгоднее при `k` близком к числу различных значений и при `k = 1`?
6. Что вы можете использовать вместо `unordered_map`, если значения лежат в узком диапазоне (здесь 20001 значение)?
7. Как изменится решение, если ответ не единственный (по условию) и нужно выбрать любые `k`?

**Целевая сложность:** O(n) по времени (корзины) либо O(n log k) (куча), память O(n).

**Шаблон**

```cpp
namespace {
	std::vector<int> top_k_frequent([[maybe_unused]] const std::vector<int>& nums,
									[[maybe_unused]] const int k) {
		return {};
	}

	void test_top_k_frequent(const std::string& label,
							 const std::vector<int>& nums,
							 const int k,
							 std::vector<int> expected) {
		auto result{top_k_frequent(nums, k)};
		std::ranges::sort(result);
		std::ranges::sort(expected);
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_top_k_frequent("description example 1",
		{1, 1, 1, 2, 2, 3},
		2,
		{1, 2});
	test_top_k_frequent("description example 2",
		{1},
		1,
		{1});
	test_top_k_frequent("single value many times",
		{4, 4, 4, 4},
		1,
		{4});
	test_top_k_frequent("negative single value",
		{-3},
		1,
		{-3});
	test_top_k_frequent("all values equally frequent, k = all",
		{1, 2, 3},
		3,
		{1, 2, 3});
	test_top_k_frequent("all values equally frequent twice, k = all",
		{5, 5, 6, 6, 7, 7},
		3,
		{5, 6, 7});
	test_top_k_frequent("k = 1, clear winner",
		{1, 2, 2, 3, 3, 3},
		1,
		{3});
	test_top_k_frequent("k = 2, winners at the end",
		{1, 2, 2, 3, 3, 3, 4, 4, 4, 4},
		2,
		{3, 4});
	test_top_k_frequent("k = 2, winners at the start",
		{9, 9, 9, 9, 8, 8, 8, 1, 2},
		2,
		{8, 9});
	test_top_k_frequent("ties below the cutoff",
		{1, 1, 1, 2, 2, 2, 3, 4, 5},
		2,
		{1, 2});
	test_top_k_frequent("two equally frequent values, both are taken",
		{1, 1, 2, 2, 3},
		2,
		{1, 2});
	test_top_k_frequent("negative values",
		{-1, -1, -1, -2, -2, -3},
		2,
		{-2, -1});
	test_top_k_frequent("negative and positive values",
		{-5, 5, 5, -5, -5, 0, 0, 0, 0},
		2,
		{-5, 0});
	test_top_k_frequent("zero is the most frequent",
		{0, 0, 0, 1, 1, 2},
		1,
		{0});
	test_top_k_frequent("bounds of the value range",
		{10000, 10000, 10000, -10000, -10000, 0},
		2,
		{-10000, 10000});
	test_top_k_frequent("frequencies 1..5, k = 3",
		{1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5},
		3,
		{3, 4, 5});
	test_top_k_frequent("frequencies 1..5, k = 4",
		{1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5},
		4,
		{2, 3, 4, 5});
	test_top_k_frequent("scrambled order",
		{3, 1, 2, 3, 1, 3, 2, 3, 4, 1, 3},
		1,
		{3});
	test_top_k_frequent("most frequent value arrives last",
		{1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7},
		1,
		{7});
	test_top_k_frequent("many singletons and one pair",
		{1, 2, 3, 4, 5, 6, 6},
		1,
		{6});
	test_top_k_frequent("k equals the number of distinct values",
		{1, 1, 2, 3, 3, 3},
		3,
		{1, 2, 3});
	test_top_k_frequent("random 1, n=27, k=2",
		{-4, -4, 2, 0, 4, 4, 4, 0, 5, 2, 0, 0, 5, -2, 3, -2, -1, -2, 1, 4, 3, 3, 2, -3, 1, -3, 0},
		2,
		{0, 4});
	test_top_k_frequent("random 2, n=36, k=13",
		{-1, -1, -3, 3, -4, 2, -3, -6, 1, 0, 2, 3, 5, 1, 1, 1, 6, 5, 1, 1, -1, 5, 2, 3, 5, -6, -3, -2, 4, 4, 1, -4, -5, 4, -2, 4},
		13,
		{-6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6});
	test_top_k_frequent("random 3, n=35, k=2",
		{6, 1, 1, 5, 3, -6, -3, -5, 1, 2, -4, -6, 4, -5, 4, 1, 5, 4, 2, -1, -1, 3, -1, -1, -6, 3, 4, 5, 4, -2, -1, -3, -2, -2, -6},
		2,
		{-1, 4});
	test_top_k_frequent("random 4, n=21, k=12",
		{6, 4, 3, -3, -2, 5, 6, 2, -3, 1, 2, -3, 5, -5, -6, 6, -6, -1, -4, -3, -2},
		12,
		{-6, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 6});

	{
		// n = 100000, one value only -> that value
		const std::vector<int> nums(100000, -7777);
		test_top_k_frequent("n = 100000, one value",
			nums,
			1,
			{-7777});
	}
	{
		// n = 100000, i % 20001 - 10000: the first 19996 values appear 5 times, the last 5 values 4 times
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i % 20001) - 10000;
		}
		std::vector<int> expected;
		for (int v{-10000}; v < -10000 + 19996; ++v) expected.push_back(v);
		test_top_k_frequent("n = 100000, 20001 values, cutoff between 5 and 4",
			nums,
			19996,
			expected);
	}
	{
		// value j (j = 0..445) appears j + 1 times, scrambled order, k = 10 -> 436..445
		std::vector<int> base;
		for (int j{0}; j <= 445; ++j) {
			for (int c{0}; c <= j; ++c) base.push_back(j);
		}
		std::vector<int> nums(base.size());
		for (std::size_t i{0}; i < base.size(); ++i) {
			nums[i * 7919 % base.size()] = base[i];
		}
		std::vector<int> expected;
		for (int v{436}; v <= 445; ++v) expected.push_back(v);
		test_top_k_frequent("n = 99681, staircase frequencies, k = 10",
			nums,
			10,
			expected);
	}
	{
		// all 20001 values once, k = 20001 -> all of them
		std::vector<int> nums;
		for (int v{10000}; v >= -10000; --v) nums.push_back(v);
		std::vector<int> expected(nums.rbegin(), nums.rend());
		test_top_k_frequent("n = 20001, all values once, k = all",
			nums,
			20001,
			expected);
	}
	{
		// 50000 x 1, 30000 x 2, 20000 x 3 -> k = 1 is 1, k = 2 is {1, 2}
		std::vector<int> nums;
		nums.insert(nums.end(), 50000, 1);
		nums.insert(nums.end(), 30000, 2);
		nums.insert(nums.end(), 20000, 3);
		test_top_k_frequent("n = 100000, three values, k = 1", nums, 1, {1});
		test_top_k_frequent("n = 100000, three values, k = 2", nums, 2, {1, 2});
	}
	{
		// the most frequent value is the largest and arrives last
		std::vector<int> nums;
		for (int v{-10000}; v < 10000; ++v) nums.push_back(v);
		nums.insert(nums.end(), 80000, 10000);
		test_top_k_frequent("n = 100000, 20000 singletons and one value x 80000, k = 1",
			nums,
			1,
			{10000});
	}

	return 0;
}
```

Порядок элементов в вашем ответе тест не учитывает: он сортирует и ваш результат, и ожидаемый. Пришлите свою попытку, и я её разберу.

---

work: 12:56 - 13:31

---

- 1, 1, 1, 2, 2, 3 | k=2
- 1, 1, 1, 2, 2, 3 & sort
- 1, 1, 1, [2], 2, 3 | use upper_bound, calc distance {3: {1}} | pq{(1, 3)}
- 1, 1, 1, 2, 2, [3] | use upper_bound, calc distance {3: {1}, 2: {2}} | pq{(1, 3), (2, 2)}
- 1, 1, 1, 2, 2, 3 [] | use upper_bound, calc distance {3: {1}, 2: {2}, 1: {3}} | pq{(1, 3), (2, 2)}

- 1, 1, 1, 2, 2, 3 | k=2
- ...
- 1, 1, 1, [2], 2, 3 | k=2, map{3: {1}} pq{3}
- 1, 1, 1, 2, 2, [3] | k=2, map{3: {1}, 2: {2}} pq{3, 2}
- 1, 1, 1, 2, 2, 3 [] | k=2, map{3: {1}, 2: {2}, 1: {3}} pq{3, 2, 1}


---

```cpp
#include <cstddef>
#include <iostream>
#include <format>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <algorithm>

namespace {
	using Pair = std::pair<int, int>;

	struct Comparator {
		bool operator()(const Pair& lhs, const Pair& rhs) const {
			return lhs.first > rhs.first;
		}
	};

	std::vector<int> top_k_frequent(const std::vector<int>& nums,
									const int k) {
		
		std::unordered_map<int, int> counter_by_value;
		for (const auto num: nums) {
			++counter_by_value[num];
		}
		
		const auto size{static_cast<std::size_t>(k)};
		std::priority_queue<Pair, std::vector<Pair>, Comparator> min_pq;
		for (const auto [value, count]: counter_by_value) {
			min_pq.push({count, value});
			if (min_pq.size() > size) min_pq.pop();
		}

		std::vector<int> result;
		result.reserve(size);
		while (!min_pq.empty())  {
			result.push_back(min_pq.top().second);
			min_pq.pop();
		}
		std::ranges::reverse(result);

		return result;
	}

	void test_top_k_frequent(const std::string& label,
							 const std::vector<int>& nums,
							 const int k,
							 std::vector<int> expected) {
		auto result{top_k_frequent(nums, k)};
		std::ranges::sort(result);
		std::ranges::sort(expected);
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_top_k_frequent("description example 1",
		{1, 1, 1, 2, 2, 3},
		2,
		{1, 2});
	test_top_k_frequent("description example 2",
		{1},
		1,
		{1});
	test_top_k_frequent("single value many times",
		{4, 4, 4, 4},
		1,
		{4});
	test_top_k_frequent("negative single value",
		{-3},
		1,
		{-3});
	test_top_k_frequent("all values equally frequent, k = all",
		{1, 2, 3},
		3,
		{1, 2, 3});
	test_top_k_frequent("all values equally frequent twice, k = all",
		{5, 5, 6, 6, 7, 7},
		3,
		{5, 6, 7});
	test_top_k_frequent("k = 1, clear winner",
		{1, 2, 2, 3, 3, 3},
		1,
		{3});
	test_top_k_frequent("k = 2, winners at the end",
		{1, 2, 2, 3, 3, 3, 4, 4, 4, 4},
		2,
		{3, 4});
	test_top_k_frequent("k = 2, winners at the start",
		{9, 9, 9, 9, 8, 8, 8, 1, 2},
		2,
		{8, 9});
	test_top_k_frequent("ties below the cutoff",
		{1, 1, 1, 2, 2, 2, 3, 4, 5},
		2,
		{1, 2});
	test_top_k_frequent("two equally frequent values, both are taken",
		{1, 1, 2, 2, 3},
		2,
		{1, 2});
	test_top_k_frequent("negative values",
		{-1, -1, -1, -2, -2, -3},
		2,
		{-2, -1});
	test_top_k_frequent("negative and positive values",
		{-5, 5, 5, -5, -5, 0, 0, 0, 0},
		2,
		{-5, 0});
	test_top_k_frequent("zero is the most frequent",
		{0, 0, 0, 1, 1, 2},
		1,
		{0});
	test_top_k_frequent("bounds of the value range",
		{10000, 10000, 10000, -10000, -10000, 0},
		2,
		{-10000, 10000});
	test_top_k_frequent("frequencies 1..5, k = 3",
		{1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5},
		3,
		{3, 4, 5});
	test_top_k_frequent("frequencies 1..5, k = 4",
		{1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5},
		4,
		{2, 3, 4, 5});
	test_top_k_frequent("scrambled order",
		{3, 1, 2, 3, 1, 3, 2, 3, 4, 1, 3},
		1,
		{3});
	test_top_k_frequent("most frequent value arrives last",
		{1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7},
		1,
		{7});
	test_top_k_frequent("many singletons and one pair",
		{1, 2, 3, 4, 5, 6, 6},
		1,
		{6});
	test_top_k_frequent("k equals the number of distinct values",
		{1, 1, 2, 3, 3, 3},
		3,
		{1, 2, 3});
	test_top_k_frequent("random 1, n=27, k=2",
		{-4, -4, 2, 0, 4, 4, 4, 0, 5, 2, 0, 0, 5, -2, 3, -2, -1, -2, 1, 4, 3, 3, 2, -3, 1, -3, 0},
		2,
		{0, 4});
	test_top_k_frequent("random 2, n=36, k=13",
		{-1, -1, -3, 3, -4, 2, -3, -6, 1, 0, 2, 3, 5, 1, 1, 1, 6, 5, 1, 1, -1, 5, 2, 3, 5, -6, -3, -2, 4, 4, 1, -4, -5, 4, -2, 4},
		13,
		{-6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6});
	test_top_k_frequent("random 3, n=35, k=2",
		{6, 1, 1, 5, 3, -6, -3, -5, 1, 2, -4, -6, 4, -5, 4, 1, 5, 4, 2, -1, -1, 3, -1, -1, -6, 3, 4, 5, 4, -2, -1, -3, -2, -2, -6},
		2,
		{-1, 4});
	test_top_k_frequent("random 4, n=21, k=12",
		{6, 4, 3, -3, -2, 5, 6, 2, -3, 1, 2, -3, 5, -5, -6, 6, -6, -1, -4, -3, -2},
		12,
		{-6, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 6});

	{
		// n = 100000, one value only -> that value
		const std::vector<int> nums(100000, -7777);
		test_top_k_frequent("n = 100000, one value",
			nums,
			1,
			{-7777});
	}
	{
		// n = 100000, i % 20001 - 10000: the first 19996 values appear 5 times, the last 5 values 4 times
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i % 20001) - 10000;
		}
		std::vector<int> expected;
		for (int v{-10000}; v < -10000 + 19996; ++v) expected.push_back(v);
		test_top_k_frequent("n = 100000, 20001 values, cutoff between 5 and 4",
			nums,
			19996,
			expected);
	}
	{
		// value j (j = 0..445) appears j + 1 times, scrambled order, k = 10 -> 436..445
		std::vector<int> base;
		for (int j{0}; j <= 445; ++j) {
			for (int c{0}; c <= j; ++c) base.push_back(j);
		}
		std::vector<int> nums(base.size());
		for (std::size_t i{0}; i < base.size(); ++i) {
			nums[i * 7919 % base.size()] = base[i];
		}
		std::vector<int> expected;
		for (int v{436}; v <= 445; ++v) expected.push_back(v);
		test_top_k_frequent("n = 99681, staircase frequencies, k = 10",
			nums,
			10,
			expected);
	}
	{
		// all 20001 values once, k = 20001 -> all of them
		std::vector<int> nums;
		for (int v{10000}; v >= -10000; --v) nums.push_back(v);
		std::vector<int> expected(nums.rbegin(), nums.rend());
		test_top_k_frequent("n = 20001, all values once, k = all",
			nums,
			20001,
			expected);
	}
	{
		// 50000 x 1, 30000 x 2, 20000 x 3 -> k = 1 is 1, k = 2 is {1, 2}
		std::vector<int> nums;
		nums.insert(nums.end(), 50000, 1);
		nums.insert(nums.end(), 30000, 2);
		nums.insert(nums.end(), 20000, 3);
		test_top_k_frequent("n = 100000, three values, k = 1", nums, 1, {1});
		test_top_k_frequent("n = 100000, three values, k = 2", nums, 2, {1, 2});
	}
	{
		// the most frequent value is the largest and arrives last
		std::vector<int> nums;
		for (int v{-10000}; v < 10000; ++v) nums.push_back(v);
		nums.insert(nums.end(), 80000, 10000);
		test_top_k_frequent("n = 100000, 20000 singletons and one value x 80000, k = 1",
			nums,
			1,
			{10000});
	}

	return 0;
}
```
