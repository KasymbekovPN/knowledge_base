## 128. Longest Consecutive Sequence (Medium)

Дан неотсортированный массив целых чисел `nums`. Нужно вернуть длину самой длинной последовательности **подряд идущих значений** (например, `5, 6, 7, 8`). Элементы последовательности не обязаны стоять в массиве рядом или в порядке возрастания. Повторяющиеся значения считаются один раз.

**Примеры**

- `nums = [100,4,200,1,3,2]` → `4` (последовательность `1, 2, 3, 4`)
- `nums = [0,3,7,2,5,8,4,6,0,1]` → `9` (`0..8`)
- `nums = [1,0,1,2]` → `3` (`0, 1, 2`)

**Сигнатура**

```cpp
int longest_consecutive(const std::vector<int>& nums);
```

**Ограничения**

- `0 <= nums.length <= 10^5`
- `-10^9 <= nums[i] <= 10^9`

**Вопросы интервьюера**

1. Какое решение через сортировку и какая у него сложность? Как оно обрабатывает повторяющиеся значения?
2. Требуется O(n): как этого добиться без сортировки?
3. Почему обход «от каждого элемента вверх» может стать O(n²)? Как этого избежать, не выполняя лишней работы?
4. Как определить, что элемент — начало последовательности?
5. Что делать с повторами и пустым массивом? Может ли `x - 1` или `x + 1` выйти за пределы `int` при данных ограничениях?
6. Какую структуру выбрать: `unordered_set`, `unordered_map` (границы отрезков) или DSU? Каковы компромиссы?
7. Как защититься от антихеш-тестов?

**Целевая сложность:** O(n) по времени в среднем, O(n) по памяти.

**Шаблон**

```cpp
namespace {
	int longest_consecutive([[maybe_unused]] const std::vector<int>& nums) {
		return 0;
	}

	void test_longest_consecutive(const std::string& label,
								  const std::vector<int>& nums,
								  const int expected) {
		const auto result{longest_consecutive(nums)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_longest_consecutive("description example 1",
		{100, 4, 200, 1, 3, 2},
		4);
	test_longest_consecutive("description example 2",
		{0, 3, 7, 2, 5, 8, 4, 6, 0, 1},
		9);
	test_longest_consecutive("description example 3",
		{1, 0, 1, 2},
		3);
	test_longest_consecutive("empty array",
		{},
		0);
	test_longest_consecutive("single element",
		{7},
		1);
	test_longest_consecutive("single negative element",
		{-7},
		1);
	test_longest_consecutive("two equal elements",
		{5, 5},
		1);
	test_longest_consecutive("two consecutive elements",
		{5, 6},
		2);
	test_longest_consecutive("two consecutive elements, reversed",
		{6, 5},
		2);
	test_longest_consecutive("two elements with a gap",
		{5, 7},
		1);
	test_longest_consecutive("all equal",
		{3, 3, 3, 3, 3},
		1);
	test_longest_consecutive("sorted ascending",
		{1, 2, 3, 4, 5},
		5);
	test_longest_consecutive("sorted descending",
		{5, 4, 3, 2, 1},
		5);
	test_longest_consecutive("duplicates inside a sequence",
		{1, 2, 2, 3, 3, 3, 4},
		4);
	test_longest_consecutive("duplicates must not extend the sequence",
		{1, 1, 1, 1, 2, 2},
		2);
	test_longest_consecutive("every other number",
		{1, 3, 5, 7, 9},
		1);
	test_longest_consecutive("gap of two between sequences",
		{1, 2, 3, 6, 7, 8, 9},
		4);
	test_longest_consecutive("longest sequence is first",
		{1, 2, 3, 4, 10, 11},
		4);
	test_longest_consecutive("longest sequence is last",
		{1, 2, 10, 11, 12, 13},
		4);
	test_longest_consecutive("two sequences of equal length",
		{1, 2, 3, 10, 11, 12},
		3);
	test_longest_consecutive("sequence crosses zero",
		{-2, -1, 0, 1, 2},
		5);
	test_longest_consecutive("negative sequence",
		{-5, -3, -4, -1, -2},
		5);
	test_longest_consecutive("zero only",
		{0},
		1);
	test_longest_consecutive("zeros and ones",
		{0, 0, 1, 1, 0},
		2);
	test_longest_consecutive("minus one and one, but not zero",
		{-1, 1},
		1);
	test_longest_consecutive("extreme values, far apart",
		{-1000000000, 1000000000},
		1);
	test_longest_consecutive("extreme values, both ends of a sequence",
		{1000000000, 999999999, 999999998},
		3);
	test_longest_consecutive("extreme low values",
		{-1000000000, -999999999, -999999998, -999999997},
		4);
	test_longest_consecutive("extreme values, no overflow at limits",
		{1000000000, -1000000000, 0},
		1);
	test_longest_consecutive("middle element arrives last",
		{1, 2, 4, 5, 3},
		5);
	test_longest_consecutive("sequence formed by three interleaved parts",
		{10, 1, 9, 2, 8, 3, 7, 4, 6, 5},
		10);
	test_longest_consecutive("two sequences joined by the last element",
		{1, 2, 3, 5, 6, 7, 4},
		7);
	test_longest_consecutive("random 1, n=15",
		{2, 6, 1, -6, 5, -10, -6, 0, 10, 7, -2, 10, 8, 7, -1},
		5);
	test_longest_consecutive("random 2, n=10",
		{-7, -10, -1, 1, -7, 10, 4, -5, 2, 6},
		2);
	test_longest_consecutive("random 3, n=19",
		{5, 0, 5, 0, 6, 1, 3, -1, -2, -4, 8, -10, -6, 8, -6, -8, -8, 8, 4},
		4);
	test_longest_consecutive("random 4, n=16",
		{4, 6, 2, 0, 8, 3, 0, -9, 2, -6, 4, -10, 3, 4, 6, -2},
		3);
	test_longest_consecutive("random wide values 1, n=13",
		{42526108, 733542908, -150623516, 646802293, 683693276, 187530162, -114157183, -519616338, 762965125, -515679980, -92576252, -643465926, 446350265},
		1);
	test_longest_consecutive("random wide values 2, n=8",
		{60844075, 662326696, -757334623, 961357309, 217564378, -446004591, -898221101, 406929851},
		1);

	{
		// values 0..99999 in a scrambled order -> one run of 100000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i * 7919 % 100000);
		}
		test_longest_consecutive("n = 100000, shuffled 0..99999",
			nums,
			100000);
	}
	{
		// values -50000..49999 in a scrambled order -> one run of 100000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i * 7919 % 100000) - 50000;
		}
		test_longest_consecutive("n = 100000, shuffled run crossing zero",
			nums,
			100000);
	}
	{
		// values -10^9..-10^9+99999 in a scrambled order -> one run of 100000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i * 7919 % 100000) - 1000000000;
		}
		test_longest_consecutive("n = 100000, shuffled run at the lower limit",
			nums,
			100000);
	}
	{
		// values 10^9-99999..10^9 in a scrambled order -> one run of 100000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = 1000000000 - 99999 + static_cast<int>(i * 7919 % 100000);
		}
		test_longest_consecutive("n = 100000, shuffled run at the upper limit",
			nums,
			100000);
	}
	{
		// all equal -> 1
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = 123456789;
		}
		test_longest_consecutive("n = 100000, all equal",
			nums,
			1);
	}
	{
		// 0, 2, 4, ... -> 1
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i) * 2;
		}
		test_longest_consecutive("n = 100000, even numbers only",
			nums,
			1);
	}
	{
		// 0..49999 and 100000..149999 in a scrambled order -> 50000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			const int j{static_cast<int>(i * 7919 % 100000)};
			nums[i] = j < 50000 ? j : j + 50000;
		}
		test_longest_consecutive("n = 100000, two runs of 50000 with a gap",
			nums,
			50000);
	}
	{
		// each of 0..49999 appears twice -> 50000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i * 7919 % 50000);
		}
		test_longest_consecutive("n = 100000, every value of a run twice",
			nums,
			50000);
	}

	return 0;
}
```

Решение «от каждого элемента вверх» без проверки начала последовательности проходит все малые тесты, но на больших зависает: у меня оно не уложилось и в две минуты. Пришлите свою попытку, и я её разберу.

---

work: 17:33 - 17:59

---

- 100,4,200, 4,1,3,2
- 1, 2, 3, 4, 4,100, 200
- [1], 2, 3, 4, 4, 100, 200 => counter = 1
- ...
- [1, 2, 3, 4], 4, 100, 200 => counter = 4
- [1, 2, 3, 4, 4],100, 200 => counter = 4 (do not inc)
- 1, 2, 3, 4, 4,[100], 200 => counter = 1, best_counter = 4
- 1, 2, 3, 4, 4,[100, 200] => counter = 2, best_counter = 4

- 100, 4, 200, 4, 1, 3, 2 => set = {100}
- 100, 4, 200, 4, 1, 3, 2 => set = {100, 4} counter = 2 best = 2
- 100, 4, 200, 4, 1, 3, 2 => set = {100, 4, 200} counter = 1 best = 1
- 100, 4, 200, 4, 1, 3, 2 => set = {100, 4, 200} counter = 1 best = 1
- 100, 4, 200, 4, 1, 3, 2 => set = {100, 4, 200, 1} counter = 1 best = 1
- 100, 4, 200, 4, 1, 3, 2 => set = {100, 4, 200, 1, 2} counter = 2 best = 2
- 100, 4, 200, 4, 1, 3, 2 => set = {100, 4, 200, 1, 2, 3} counter = 4 best = 4

---

```cpp
#include <iostream>
#include <format>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cstddef>

namespace {
	[[nodiscard]] int longest_consecutive_sort(std::vector<int> nums) {
		const std::size_t size{nums.size()};
		if (size < 2) return static_cast<int>(size);
		
		std::ranges::sort(nums);

		int best{0};
		int counter{1};
		for (std::size_t i{1}; i < size; ++i) {
			if (nums[i-1] + 1 == nums[i]) {
				++counter;
			} else if (nums[i-1] != nums[i]) {
				counter = 1;
			}
			
			best = std::max(best, counter);
		}
	
		return best;
	}
	
	[[nodiscard]] int longest_consecutive_set(const std::vector<int>& nums) {
		const std::size_t size{nums.size()};
		if (size < 2) return static_cast<int>(size);

		std::unordered_set<int> buffer(nums.begin(), nums.end());
		
		int best{0};
		int counter{0};
		for (const auto num: buffer) {
			if (buffer.contains(num - 1)) continue;
			
			counter = 1;
			while (buffer.contains(num + counter)) ++counter;
			best = std::max(best, counter);
		}

		return best;
	}

	void test_longest_consecutive(const std::string& label,
								  const std::vector<int>& nums,
								  const int expected) {
		const auto result_sort{longest_consecutive_sort(nums)};
		const auto result_set{longest_consecutive_set(nums)};
		std::cout << std::format("{}: sort: {}, set: {}\n",
			label,
			result_sort == expected ? "OK" : "FAIL",
			result_set == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_longest_consecutive("description example 1",
		{100, 4, 200, 1, 3, 2},
		4);
	test_longest_consecutive("description example 2",
		{0, 3, 7, 2, 5, 8, 4, 6, 0, 1},
		9);
	test_longest_consecutive("description example 3",
		{1, 0, 1, 2},
		3);
	test_longest_consecutive("empty array",
		{},
		0);
	test_longest_consecutive("single element",
		{7},
		1);
	test_longest_consecutive("single negative element",
		{-7},
		1);
	test_longest_consecutive("two equal elements",
		{5, 5},
		1);
	test_longest_consecutive("two consecutive elements",
		{5, 6},
		2);
	test_longest_consecutive("two consecutive elements, reversed",
		{6, 5},
		2);
	test_longest_consecutive("two elements with a gap",
		{5, 7},
		1);
	test_longest_consecutive("all equal",
		{3, 3, 3, 3, 3},
		1);
	test_longest_consecutive("sorted ascending",
		{1, 2, 3, 4, 5},
		5);
	test_longest_consecutive("sorted descending",
		{5, 4, 3, 2, 1},
		5);
	test_longest_consecutive("duplicates inside a sequence",
		{1, 2, 2, 3, 3, 3, 4},
		4);
	test_longest_consecutive("duplicates must not extend the sequence",
		{1, 1, 1, 1, 2, 2},
		2);
	test_longest_consecutive("every other number",
		{1, 3, 5, 7, 9},
		1);
	test_longest_consecutive("gap of two between sequences",
		{1, 2, 3, 6, 7, 8, 9},
		4);
	test_longest_consecutive("longest sequence is first",
		{1, 2, 3, 4, 10, 11},
		4);
	test_longest_consecutive("longest sequence is last",
		{1, 2, 10, 11, 12, 13},
		4);
	test_longest_consecutive("two sequences of equal length",
		{1, 2, 3, 10, 11, 12},
		3);
	test_longest_consecutive("sequence crosses zero",
		{-2, -1, 0, 1, 2},
		5);
	test_longest_consecutive("negative sequence",
		{-5, -3, -4, -1, -2},
		5);
	test_longest_consecutive("zero only",
		{0},
		1);
	test_longest_consecutive("zeros and ones",
		{0, 0, 1, 1, 0},
		2);
	test_longest_consecutive("minus one and one, but not zero",
		{-1, 1},
		1);
	test_longest_consecutive("extreme values, far apart",
		{-1000000000, 1000000000},
		1);
	test_longest_consecutive("extreme values, both ends of a sequence",
		{1000000000, 999999999, 999999998},
		3);
	test_longest_consecutive("extreme low values",
		{-1000000000, -999999999, -999999998, -999999997},
		4);
	test_longest_consecutive("extreme values, no overflow at limits",
		{1000000000, -1000000000, 0},
		1);
	test_longest_consecutive("middle element arrives last",
		{1, 2, 4, 5, 3},
		5);
	test_longest_consecutive("sequence formed by three interleaved parts",
		{10, 1, 9, 2, 8, 3, 7, 4, 6, 5},
		10);
	test_longest_consecutive("two sequences joined by the last element",
		{1, 2, 3, 5, 6, 7, 4},
		7);
	test_longest_consecutive("random 1, n=15",
		{2, 6, 1, -6, 5, -10, -6, 0, 10, 7, -2, 10, 8, 7, -1},
		5);
	test_longest_consecutive("random 2, n=10",
		{-7, -10, -1, 1, -7, 10, 4, -5, 2, 6},
		2);
	test_longest_consecutive("random 3, n=19",
		{5, 0, 5, 0, 6, 1, 3, -1, -2, -4, 8, -10, -6, 8, -6, -8, -8, 8, 4},
		4);
	test_longest_consecutive("random 4, n=16",
		{4, 6, 2, 0, 8, 3, 0, -9, 2, -6, 4, -10, 3, 4, 6, -2},
		3);
	test_longest_consecutive("random wide values 1, n=13",
		{42526108, 733542908, -150623516, 646802293, 683693276, 187530162, -114157183, -519616338, 762965125, -515679980, -92576252, -643465926, 446350265},
		1);
	test_longest_consecutive("random wide values 2, n=8",
		{60844075, 662326696, -757334623, 961357309, 217564378, -446004591, -898221101, 406929851},
		1);

	{
		// values 0..99999 in a scrambled order -> one run of 100000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i * 7919 % 100000);
		}
		test_longest_consecutive("n = 100000, shuffled 0..99999",
			nums,
			100000);
	}
	{
		// values -50000..49999 in a scrambled order -> one run of 100000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i * 7919 % 100000) - 50000;
		}
		test_longest_consecutive("n = 100000, shuffled run crossing zero",
			nums,
			100000);
	}
	{
		// values -10^9..-10^9+99999 in a scrambled order -> one run of 100000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i * 7919 % 100000) - 1000000000;
		}
		test_longest_consecutive("n = 100000, shuffled run at the lower limit",
			nums,
			100000);
	}
	{
		// values 10^9-99999..10^9 in a scrambled order -> one run of 100000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = 1000000000 - 99999 + static_cast<int>(i * 7919 % 100000);
		}
		test_longest_consecutive("n = 100000, shuffled run at the upper limit",
			nums,
			100000);
	}
	{
		// all equal -> 1
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = 123456789;
		}
		test_longest_consecutive("n = 100000, all equal",
			nums,
			1);
	}
	{
		// 0, 2, 4, ... -> 1
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i) * 2;
		}
		test_longest_consecutive("n = 100000, even numbers only",
			nums,
			1);
	}
	{
		// 0..49999 and 100000..149999 in a scrambled order -> 50000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			const int j{static_cast<int>(i * 7919 % 100000)};
			nums[i] = j < 50000 ? j : j + 50000;
		}
		test_longest_consecutive("n = 100000, two runs of 50000 with a gap",
			nums,
			50000);
	}
	{
		// each of 0..49999 appears twice -> 50000
		std::vector<int> nums(100000);
		for (std::size_t i{0}; i < nums.size(); ++i) {
			nums[i] = static_cast<int>(i * 7919 % 50000);
		}
		test_longest_consecutive("n = 100000, every value of a run twice",
			nums,
			50000);
	}

	return 0;
}
```
