**Задача 1 блока 5 (бинарный поиск): Find First and Last Position**

Предыдущий шаблон был неполным, вот правильный и полный.

**Условие.** Дан массив целых чисел `nums`, отсортированный по неубыванию, и число `target`. Верните индексы первого и последнего вхождения `target` в виде `{first, last}`. Если `target` в массиве нет, верните `{-1, -1}`.

**Сигнатура:**

```cpp
std::vector<int> search_range(const std::vector<int>& nums, int target);
```

**Ограничения:**

- 0 ≤ n ≤ 1 000 000
- -10^9 ≤ nums[i], target ≤ 10^9
- массив отсортирован по неубыванию, возможны повторы

**Примеры:**

- `nums = [5, 7, 7, 8, 8, 10], target = 8` → `{3, 4}`
- `nums = [5, 7, 7, 8, 8, 10], target = 6` → `{-1, -1}`
- `nums = [], target = 0` → `{-1, -1}`

**Вопросы интервьюера (ответьте после решения):**

1. Какая сложность по времени у вашего решения? Что произойдёт на массиве из миллиона одинаковых чисел, если после нахождения одного вхождения расширять границы шагами влево и вправо?
2. Какой инвариант у вашего цикла: какой смысл у `lo` и `hi` на каждой итерации и почему цикл завершается?
3. Как вы считаете середину и почему именно так? Может ли здесь быть переполнение при этих ограничениях?
4. Можно ли обойтись двумя вызовами одной и той же функции поиска границы? Чем будут отличаться условия?

**Цель:** O(log n) по времени, O(1) по памяти.

**Шаблон:**

```cpp
namespace {
	std::vector<int> search_range([[maybe_unused]] const std::vector<int>& nums,
								  [[maybe_unused]] int target) {
		return {};
	}

	std::vector<int> expected_range(const std::vector<int>& nums, int target) {
		const auto [first, last] = std::ranges::equal_range(nums, target);
		if (first == last) return {-1, -1};
		return {static_cast<int>(first - nums.begin()),
				static_cast<int>(last - nums.begin()) - 1};
	}

	void test_search(const std::string& label,
					 const std::vector<int>& nums,
					 int target) {
		const auto expected{expected_range(nums, target)};

		const auto result{search_range(nums, target)};

		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}

	// many queries against one array: a linear scan cannot pass this
	void test_search_many(const std::string& label,
						  const std::vector<int>& nums,
						  const std::vector<int>& targets) {
		bool ok{true};
		for (const int target : targets) {
			if (search_range(nums, target) != expected_range(nums, target)) {
				ok = false;
				break;
			}
		}

		std::cout << std::format("{}: {}\n",
			label,
			ok ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_search("description example 1",
		{5, 7, 7, 8, 8, 10},
		8);
	test_search("description example 2",
		{5, 7, 7, 8, 8, 10},
		6);
	test_search("description example 3: empty array",
		{},
		0);
	test_search("single element, found",
		{5},
		5);
	test_search("single element, smaller target",
		{5},
		4);
	test_search("single element, greater target",
		{5},
		6);
	test_search("two equal elements",
		{3, 3},
		3);
	test_search("two different, first",
		{1, 2},
		1);
	test_search("two different, second",
		{1, 2},
		2);
	test_search("two different, missing between",
		{1, 3},
		2);
	test_search("target smaller than all",
		{2, 4, 6, 8},
		1);
	test_search("target greater than all",
		{2, 4, 6, 8},
		9);
	test_search("target between elements",
		{2, 4, 6, 8},
		5);
	test_search("first element",
		{2, 4, 6, 8},
		2);
	test_search("last element",
		{2, 4, 6, 8},
		8);
	test_search("all equal, found",
		{7, 7, 7, 7, 7},
		7);
	test_search("all equal, smaller target",
		{7, 7, 7, 7, 7},
		6);
	test_search("all equal, greater target",
		{7, 7, 7, 7, 7},
		8);
	test_search("run at the beginning",
		{1, 1, 1, 2, 3, 4},
		1);
	test_search("run at the end",
		{1, 2, 3, 4, 4, 4},
		4);
	test_search("run in the middle",
		{1, 2, 2, 2, 2, 3, 4},
		2);
	test_search("run in the middle, odd length run",
		{1, 2, 2, 2, 3},
		2);
	test_search("two runs, left one",
		{1, 1, 2, 2, 2, 3, 3},
		1);
	test_search("two runs, right one",
		{1, 1, 2, 2, 2, 3, 3},
		3);
	test_search("single occurrence in the middle",
		{1, 2, 3, 4, 5},
		3);
	test_search("single occurrence of a duplicated array end",
		{1, 1, 1, 2},
		2);
	test_search("single occurrence at the start",
		{1, 2, 2, 2},
		1);
	test_search("negative values, found",
		{-9, -7, -7, -3, 0, 4},
		-7);
	test_search("negative values, missing",
		{-9, -7, -7, -3, 0, 4},
		-8);
	test_search("negative target, smaller than all",
		{-9, -7, -3},
		-10);
	test_search("zero target among duplicates",
		{-1, 0, 0, 0, 1},
		0);
	test_search("extreme values, minimum",
		{-1000000000, -1000000000, 0, 1000000000},
		-1000000000);
	test_search("extreme values, maximum",
		{-1000000000, 0, 1000000000, 1000000000},
		1000000000);
	test_search("extreme values, missing",
		{-1000000000, 1000000000},
		0);
	test_search("extreme values, both ends equal",
		{-1000000000, 1000000000},
		1000000000);
	test_search("three elements, middle missing",
		{1, 1, 3},
		2);
	test_search("three equal, middle target",
		{4, 4, 4},
		4);
	test_search("long run, even length",
		{1, 5, 5, 5, 5, 5, 5, 9},
		5);
	test_search("long run, odd length",
		{1, 5, 5, 5, 5, 5, 9},
		5);
	test_search("random 1, n=9",
		{-6, -6, -5, -3, -1, -1, 0, 0, 3},
		-3);
	test_search("random 2, n=6",
		{-5, -5, -4, -2, 2, 3},
		-2);
	test_search("random 3, n=10",
		{-6, -6, -5, -4, -2, -1, 2, 6, 6, 6},
		4);
	test_search("random 4, n=9",
		{-6, -4, -2, -2, 0, 3, 4, 5, 6},
		-1);
	test_search("random 5, n=6",
		{-4, -3, -3, 1, 2, 3},
		-5);
	test_search("random 6, n=10",
		{-5, -3, -3, 0, 0, 2, 3, 4, 4, 5},
		4);
	test_search("random 7, n=2",
		{1, 6},
		-3);
	test_search("random 8, n=7",
		{-5, -4, -3, -2, 0, 3, 6},
		5);

	{
		// n = 1000000, distinct even numbers, 200000 queries (half present, half absent)
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = 2 * i;
		std::vector<int> targets;
		unsigned state{34};
		for (int i{0}; i < 200000; ++i) {
			state = state * 1103515245u + 12345u;
			targets.push_back(static_cast<int>((state >> 4) % 2100000u) - 50000);
		}
		test_search_many("n = 1000000, distinct, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, values 0..999 each repeated 1000 times, 200000 queries
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i / 1000;
		std::vector<int> targets;
		unsigned state{7};
		for (int i{0}; i < 200000; ++i) {
			state = state * 1103515245u + 12345u;
			targets.push_back(static_cast<int>((state >> 4) % 1100u) - 50);
		}
		test_search_many("n = 1000000, runs of 1000, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, all equal, 200000 queries around the value
		std::vector<int> nums(1000000, 42);
		std::vector<int> targets;
		for (int i{0}; i < 200000; ++i) targets.push_back(41 + i % 3);
		test_search_many("n = 1000000, all equal, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, only two distinct values at the extremes
		std::vector<int> nums(1000000, -1000000000);
		for (std::size_t i{500000}; i < nums.size(); ++i) nums[i] = 1000000000;
		std::vector<int> targets;
		for (int i{0}; i < 200000; ++i) {
			const int pick[]{-1000000000, 1000000000, 0, -999999999, 999999999};
			targets.push_back(pick[i % 5]);
		}
		test_search_many("n = 1000000, two extreme values, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, a short run of length 2 in the very middle
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i < 500000 ? 0 : 2;
		nums[499999] = 1;
		nums[500000] = 1;
		std::vector<int> targets;
		for (int i{0}; i < 200000; ++i) targets.push_back(i % 4);
		test_search_many("n = 1000000, short run in the middle, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, targets at the edges and beyond
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i;
		test_search("n = 1000000, target is the last element", nums, 999999);
		test_search("n = 1000000, target is the first element", nums, 0);
		test_search("n = 1000000, target is missing, too large", nums, 1000000);
	}

	return 0;
}
```

Тесты проверены запуском:

- Заглушка даёт FAIL на всех 55.
- Эталон проходит 55 из 55, санитайзеры чистые.
- Линейный поиск и «нашли одно вхождение и расширяем в стороны» упираются в время на больших тестах (миллион одинаковых чисел, много запросов).
- Вариант, который при отсутствии `target` возвращает не `{-1, -1}`, ловится на 25 тестах.
- Ошибка «последнее вхождение на единицу больше» ловится на всех 55.

Пришлите решение, и я его разберу.

---

work - 13:29 - 14:15

---

- nums.size() == 0 => ret {-1, -1}
- nums.size() == 1 => ret{-1, -1} || ret {0, -1}
- if target < nums[0] => ret {-1, -1}
- if target > nums[nums.size() - 1] => ret{-1, -1}
- if target == nums[0] && target == nums[nums.size() - 1] => ret {0, nums.size() - 1}
- took first & last elements => nums.size() * (target - first_el)  /  (last_el - first_el)
-  target = 8, nums = {5, 7, 7, 8, 8, 10},  pivot = 3
- {5, 7, 7, [8], 8, 10} =>  first_idx = 3, last_idx = 3  current == target
- {5, 7, 7, [8], [8], 10} =>  first_idx = 3, last_idx = 4  current == target

- {5, 7, 7, [8], 8, 10} pivot = 3 target = 7 idx = 3, 
- {5, [7], 7, 8, 8, 10} ret {1, 2}

- {5, 7, 7, [8], 8, 10, 10, 11} pivot = 3 target = 10
- {5, 7, 7, (8), 8, [10], 10, 11} pivot = 3 target = 10

- 5, 7, [7], 8, 8, 10 | target = 6
- 5, [7], (7), 8, 8, 10
- [5], (7), 7, 8, 8, 10  => ret {-1, -1}

- 5, 7, 7, 8, [8], 10 | target = 9
- 5, 7, 7, 8, 8, [10] |  => ret {-1, -1}


---

```cpp
#include <iostream>
#include <format>
#include <cstddef>
#include <string>
#include <vector>
#include <algorithm>

namespace {
	std::vector<int> search_range(const std::vector<int>& nums,
								  int target) {

		const std::size_t size{nums.size()};
		if (size == 0) return {-1, -1};
		if (size == 1) {
			return {
				nums[0] == target ? 0 : -1,
				-1
			};
		}
		
		if (target < nums[0]) return {-1, -1};
		if (target > nums[size - 1]) return {-1, -1};
		if (target == nums[0] && target == nums[size - 1]) {
			return {0, static_cast<int>(size - 1)};
		}

		std::size_t pivot_idx{
			(size - 1) *
			static_cast<std::size_t>(target - nums[0]) /
			static_cast<std::size_t>(nums[size - 1] - nums[0])};

		std::size_t idx{pivot_idx};
		while (idx != size) {
			const auto current_value{nums[idx]};
			if (current_value == target) {
				auto first = static_cast<int>(pivot_idx);
				auto last = static_cast<int>(pivot_idx);
				
				while (
					first != -1 &&
					nums[static_cast<std::size_t>(first)] == target
				) --first;
				++first;
				
				while (
					last != size &&
					nums[static_cast<std::size_t>(last)] == target
				) ++last;
				--last;
				
				return {first, last};
			} else {
				if (current_value < target) {
					idx += idx == size - 1 ? 1 : (size - idx) / 2;
				} else {
					// current_value > target <= stong 
					if (idx == 0) {
						return {-1, -1};
					}
					idx = idx / 2;
				}
			}
		}

		return {-1, -1};
	}
	
	std::vector<int> search_range_1(const std::vector<int>& nums,
								    int target) {

		const auto find_bound = [&nums, &target](const bool is_left) {
			std::size_t left{0};
			std::size_t right{nums.size() - 1};
			std::size_t pivot_idx{0};
			int idx{-1};

			while (left <= right) {
				pivot_idx = left + (right - left) / 2;
				if (nums[pivot_idx] > target) {
					right = pivot_idx - 1;
				} else if (nums[pivot_idx] < target) {
					left = pivot_idx + 1;
				} else {
					idx = static_cast<int>(pivot_idx);
					if (is_left) {
						right = pivot_idx - 1;
					} else {
						left = pivot_idx + 1;
					}
				}
			}
			
			return idx;
		};
		
		return {find_bound(true), find_bound(false)};
	}

	std::vector<int> expected_range(const std::vector<int>& nums, int target) {
		const auto [first, last] = std::ranges::equal_range(nums, target);
		if (first == last) return {-1, -1};
		return {static_cast<int>(first - nums.begin()),
				static_cast<int>(last - nums.begin()) - 1};
	}

	void test_search(const std::string& label,
					 const std::vector<int>& nums,
					 int target) {
		const auto expected{expected_range(nums, target)};

		const auto result{search_range(nums, target)};
		const auto result_1{search_range_1(nums, target)};

		std::cout << std::format("{}: {}, {}\n",
			label,
			result == expected ? "OK" : "FAIL",
			result_1 == expected ? "OK" : "FAIL") << std::flush;
	}

	// many queries against one array: a linear scan cannot pass this
	void test_search_many(const std::string& label,
						  const std::vector<int>& nums,
						  const std::vector<int>& targets) {
		bool ok{true};
		for (const int target : targets) {
			if (search_range(nums, target) != expected_range(nums, target)) {
				ok = false;
				break;
			}
		}

		std::cout << std::format("{}: {}\n",
			label,
			ok ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_search("description example 1",
		{5, 7, 7, 8, 8, 10},
		8);
	test_search("description example 2",
		{5, 7, 7, 8, 8, 10},
		6);
	test_search("description example 3: empty array",
		{},
		0);
	test_search("single element, found",
		{5},
		5);
	test_search("single element, smaller target",
		{5},
		4);
	test_search("single element, greater target",
		{5},
		6);
	test_search("two equal elements",
		{3, 3},
		3);
	test_search("two different, first",
		{1, 2},
		1);
	test_search("two different, second",
		{1, 2},
		2);
	test_search("two different, missing between",
		{1, 3},
		2);
	test_search("target smaller than all",
		{2, 4, 6, 8},
		1);
	test_search("target greater than all",
		{2, 4, 6, 8},
		9);
	test_search("target between elements",
		{2, 4, 6, 8},
		5);
	test_search("first element",
		{2, 4, 6, 8},
		2);
	test_search("last element",
		{2, 4, 6, 8},
		8);
	test_search("all equal, found",
		{7, 7, 7, 7, 7},
		7);
	test_search("all equal, smaller target",
		{7, 7, 7, 7, 7},
		6);
	test_search("all equal, greater target",
		{7, 7, 7, 7, 7},
		8);
	test_search("run at the beginning",
		{1, 1, 1, 2, 3, 4},
		1);
	test_search("run at the end",
		{1, 2, 3, 4, 4, 4},
		4);
	test_search("run in the middle",
		{1, 2, 2, 2, 2, 3, 4},
		2);
	test_search("run in the middle, odd length run",
		{1, 2, 2, 2, 3},
		2);
	test_search("two runs, left one",
		{1, 1, 2, 2, 2, 3, 3},
		1);
	test_search("two runs, right one",
		{1, 1, 2, 2, 2, 3, 3},
		3);
	test_search("single occurrence in the middle",
		{1, 2, 3, 4, 5},
		3);
	test_search("single occurrence of a duplicated array end",
		{1, 1, 1, 2},
		2);
	test_search("single occurrence at the start",
		{1, 2, 2, 2},
		1);
	test_search("negative values, found",
		{-9, -7, -7, -3, 0, 4},
		-7);
	test_search("negative values, missing",
		{-9, -7, -7, -3, 0, 4},
		-8);
	test_search("negative target, smaller than all",
		{-9, -7, -3},
		-10);
	test_search("zero target among duplicates",
		{-1, 0, 0, 0, 1},
		0);
	test_search("extreme values, minimum",
		{-1000000000, -1000000000, 0, 1000000000},
		-1000000000);
	test_search("extreme values, maximum",
		{-1000000000, 0, 1000000000, 1000000000},
		1000000000);
	test_search("extreme values, missing",
		{-1000000000, 1000000000},
		0);
	test_search("extreme values, both ends equal",
		{-1000000000, 1000000000},
		1000000000);
	test_search("three elements, middle missing",
		{1, 1, 3},
		2);
	test_search("three equal, middle target",
		{4, 4, 4},
		4);
	test_search("long run, even length",
		{1, 5, 5, 5, 5, 5, 5, 9},
		5);
	test_search("long run, odd length",
		{1, 5, 5, 5, 5, 5, 9},
		5);
	test_search("random 1, n=9",
		{-6, -6, -5, -3, -1, -1, 0, 0, 3},
		-3);
	test_search("random 2, n=6",
		{-5, -5, -4, -2, 2, 3},
		-2);
	test_search("random 3, n=10",
		{-6, -6, -5, -4, -2, -1, 2, 6, 6, 6},
		4);
	test_search("random 4, n=9",
		{-6, -4, -2, -2, 0, 3, 4, 5, 6},
		-1);
	test_search("random 5, n=6",
		{-4, -3, -3, 1, 2, 3},
		-5);
	test_search("random 6, n=10",
		{-5, -3, -3, 0, 0, 2, 3, 4, 4, 5},
		4);
	test_search("random 7, n=2",
		{1, 6},
		-3);
	test_search("random 8, n=7",
		{-5, -4, -3, -2, 0, 3, 6},
		5);

	{
		// n = 1000000, distinct even numbers, 200000 queries (half present, half absent)
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = 2 * i;
		std::vector<int> targets;
		unsigned state{34};
		for (int i{0}; i < 200000; ++i) {
			state = state * 1103515245u + 12345u;
			targets.push_back(static_cast<int>((state >> 4) % 2100000u) - 50000);
		}
		test_search_many("n = 1000000, distinct, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, values 0..999 each repeated 1000 times, 200000 queries
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i / 1000;
		std::vector<int> targets;
		unsigned state{7};
		for (int i{0}; i < 200000; ++i) {
			state = state * 1103515245u + 12345u;
			targets.push_back(static_cast<int>((state >> 4) % 1100u) - 50);
		}
		test_search_many("n = 1000000, runs of 1000, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, all equal, 200000 queries around the value
		std::vector<int> nums(1000000, 42);
		std::vector<int> targets;
		for (int i{0}; i < 200000; ++i) targets.push_back(41 + i % 3);
		test_search_many("n = 1000000, all equal, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, only two distinct values at the extremes
		std::vector<int> nums(1000000, -1000000000);
		for (std::size_t i{500000}; i < nums.size(); ++i) nums[i] = 1000000000;
		std::vector<int> targets;
		for (int i{0}; i < 200000; ++i) {
			const int pick[]{-1000000000, 1000000000, 0, -999999999, 999999999};
			targets.push_back(pick[i % 5]);
		}
		test_search_many("n = 1000000, two extreme values, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, a short run of length 2 in the very middle
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i < 500000 ? 0 : 2;
		nums[499999] = 1;
		nums[500000] = 1;
		std::vector<int> targets;
		for (int i{0}; i < 200000; ++i) targets.push_back(i % 4);
		test_search_many("n = 1000000, short run in the middle, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, targets at the edges and beyond
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i;
		test_search("n = 1000000, target is the last element", nums, 999999);
		test_search("n = 1000000, target is the first element", nums, 0);
		test_search("n = 1000000, target is missing, too large", nums, 1000000);
	}

	return 0;
}
```


