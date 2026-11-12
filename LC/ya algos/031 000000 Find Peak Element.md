**Задача 4 блока 5 (бинарный поиск): Find Peak Element**

Здесь массив не отсортирован, но бинарный поиск всё равно работает. Это главный вопрос задачи.

**Условие.** Пик — это элемент, который строго больше своих соседей. Дан массив `nums`, в котором соседние элементы различны (`nums[i] != nums[i + 1]`). Верните индекс любого пика. Считайте, что за границами массива с обеих сторон стоит минус бесконечность, то есть `nums[0]` пик, если `nums[0] > nums[1]`, а `nums[n - 1]` пик, если `nums[n - 1] > nums[n - 2]`. Для массива из одного элемента ответ `0`.

**Сигнатура:**

```cpp
int find_peak(const std::vector<int>& nums);
```

**Ограничения:**

- 1 ≤ n ≤ 1 000 000
- -2^31 ≤ nums[i] ≤ 2^31 - 1 (полный диапазон `int`)
- nums[i] != nums[i + 1] для всех допустимых `i`
- пиков может быть несколько, принимается любой

**Примеры:**

- `nums = [1, 2, 3, 1]` → `2` (пик 3)
- `nums = [1, 2, 1, 3, 5, 6, 4]` → `1` или `5` (пики 2 и 6)

**Вопросы интервьюера (ответьте после решения):**

1. Почему гарантированно существует хотя бы один пик? Докажите.
2. По какому правилу вы выбираете половину, и почему в выбранной половине обязательно будет пик, хотя массив не отсортирован?
3. Какие обращения к соседям вы делаете, и может ли `mid + 1` выйти за границу массива? Как проверить это в вашем цикле?
4. Как вы сравниваете соседние элементы при значениях во всём диапазоне `int`? Что произойдёт при `nums[mid + 1] - nums[mid]`?

**Цель:** O(log n) по времени, O(1) по памяти.

**Шаблон:**

```cpp
namespace {
	int find_peak([[maybe_unused]] const std::vector<int>& nums) {
		return -1;
	}

	// any peak is a correct answer: both neighbours (if they exist) are smaller
	bool is_peak(const std::vector<int>& nums, int index) {
		const int size{static_cast<int>(nums.size())};
		if (index < 0 || index >= size) return false;
		const int value{nums[static_cast<std::size_t>(index)]};
		if (index > 0 && nums[static_cast<std::size_t>(index - 1)] > value) return false;
		if (index + 1 < size && nums[static_cast<std::size_t>(index + 1)] > value) return false;
		return true;
	}

	void test_peak(const std::string& label,
				   const std::vector<int>& nums) {
		const auto result{find_peak(nums)};

		std::cout << std::format("{}: {}\n",
			label,
			is_peak(nums, result) ? "OK" : "FAIL") << std::flush;
	}

	// the same array queried many times: a linear scan cannot pass this
	void test_peak_many(const std::string& label,
						const std::vector<int>& nums,
						int repeats) {
		bool ok{true};
		for (int i{0}; i < repeats; ++i) {
			if (!is_peak(nums, find_peak(nums))) {
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
	test_peak("description example 1",
		{1, 2, 3, 1});
	test_peak("description example 2",
		{1, 2, 1, 3, 5, 6, 4});
	test_peak("single element",
		{7});
	test_peak("single element, minimum int",
		{-2147483648});
	test_peak("single element, maximum int",
		{2147483647});
	test_peak("two elements, rising",
		{1, 2});
	test_peak("two elements, falling",
		{2, 1});
	test_peak("two elements, extreme, falling",
		{2147483647, -2147483648});
	test_peak("two elements, extreme, rising",
		{-2147483648, 2147483647});
	test_peak("three elements, peak in the middle",
		{1, 3, 2});
	test_peak("three elements, valley in the middle",
		{3, 1, 2});
	test_peak("three elements, rising",
		{1, 2, 3});
	test_peak("three elements, falling",
		{3, 2, 1});
	test_peak("strictly increasing",
		{1, 2, 3, 4, 5, 6, 7, 8});
	test_peak("strictly decreasing",
		{8, 7, 6, 5, 4, 3, 2, 1});
	test_peak("mountain, peak in the middle",
		{1, 2, 3, 4, 5, 4, 3, 2, 1});
	test_peak("mountain, peak at the second position",
		{1, 9, 5, 4, 3, 2, 1});
	test_peak("mountain, peak at the second to last position",
		{1, 2, 3, 4, 5, 9, 1});
	test_peak("valley, two peaks at the ends",
		{9, 5, 1, 2, 6});
	test_peak("valley, deep",
		{10, 8, 6, 4, 2, 1, 3, 5, 7, 9});
	test_peak("zigzag starting up",
		{1, 5, 2, 6, 3, 7, 4});
	test_peak("zigzag starting down",
		{5, 1, 6, 2, 7, 3, 8});
	test_peak("zigzag, even length",
		{1, 5, 2, 6, 3, 7});
	test_peak("two peaks, the left one is higher",
		{1, 9, 2, 3, 4, 5, 4});
	test_peak("two peaks, the right one is higher",
		{1, 4, 2, 3, 4, 9, 8});
	test_peak("three peaks",
		{1, 3, 1, 5, 1, 7, 1});
	test_peak("plateau-free staircase up then down",
		{1, 2, 4, 7, 11, 16, 10, 6, 3, 1});
	test_peak("negative values, rising",
		{-9, -7, -5, -3, -1});
	test_peak("negative values, falling",
		{-1, -3, -5, -7, -9});
	test_peak("negative values, mountain",
		{-9, -5, -1, -4, -8});
	test_peak("mixed signs, zigzag",
		{-3, 4, -5, 6, -7, 8, -9});
	test_peak("extreme values alternate",
		{-2147483648, 2147483647, -2147483648, 2147483647, -2147483648});
	test_peak("extreme values alternate, starting high",
		{2147483647, -2147483648, 2147483647, -2147483648, 2147483647});
	test_peak("extreme values, rising to maximum",
		{-2147483648, 0, 2147483647});
	test_peak("extreme values, falling from maximum",
		{2147483647, 0, -2147483648});
	test_peak("extreme values, differences overflow int",
		{2147483647, -2147483648, 2147483647, -2147483648});
	test_peak("extreme values, differences overflow int, rising",
		{-2147483648, 2147483647, -2147483648, 2147483647});
	test_peak("long rise then one drop",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 19});
	test_peak("one rise then long fall",
		{1, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1});
	test_peak("random 1, n=1",
		{20});
	test_peak("random 2, n=4",
		{12, -8, 6, 11});
	test_peak("random 3, n=13",
		{19, -2, 8, 9, -19, -9, -13, 6, 0, 15, -16, 9, 16});
	test_peak("random 4, n=1",
		{-3});
	test_peak("random 5, n=2",
		{-11, 6});
	test_peak("random 6, n=8",
		{-18, -20, -7, -9, 12, -20, 19, -13});
	test_peak("random 7, n=8",
		{12, -4, -6, 15, -3, -7, 5, -5});
	test_peak("random 8, n=14",
		{-4, 5, -8, 18, 2, 20, 19, -5, -19, -4, -19, 16, -6, -20});

	{
		// n = 1000000, strictly increasing: the only peak is the last element
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i;
		test_peak_many("n = 1000000, strictly increasing, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, strictly decreasing: the only peak is the first element
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = 1000000 - i;
		test_peak_many("n = 1000000, strictly decreasing, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, mountain with the peak at 777777
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) {
			nums[static_cast<std::size_t>(i)] = i <= 777777 ? i : 1555554 - i;
		}
		test_peak_many("n = 1000000, mountain, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, valley: two peaks at the ends
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) {
			nums[static_cast<std::size_t>(i)] = i <= 500000 ? 500000 - i : i - 500000;
		}
		test_peak_many("n = 1000000, valley, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, zigzag
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = (i % 2 == 0) ? i : -i;
		test_peak_many("n = 1000000, zigzag, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, extreme values alternate: the differences overflow int
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) {
			nums[static_cast<std::size_t>(i)] = (i % 2 == 0) ? 2147483647 : -2147483647 - 1;
		}
		test_peak_many("n = 1000000, extreme alternation, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, extreme values rise in two big steps
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) {
			nums[static_cast<std::size_t>(i)] = i < 500000 ? -2147483647 - 1 + i : 2147483647 - (999999 - i);
		}
		test_peak_many("n = 1000000, extreme steps, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, pseudo-random values with different neighbours
		std::vector<int> nums(1000000);
		unsigned state{162};
		int previous{0};
		for (auto& x : nums) {
			do {
				state = state * 1103515245u + 12345u;
				x = static_cast<int>(state);
			} while (x == previous);
			previous = x;
		}
		test_peak_many("n = 1000000, random, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, sawtooth with period 1000: many peaks
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i % 1000;
		test_peak_many("n = 1000000, sawtooth, 100000 queries", nums, 100000);
	}

	return 0;
}
```

Тесты проверены запуском:

- Заглушка (возвращает `-1`) даёт FAIL на всех 56.
- Эталон проходит 56 из 56, санитайзеры чистые.
- Принимается любой пик, а не конкретный индекс.
- Неверные варианты ловятся: движение в сторону впадины (51 FAIL), `lo <= hi` с `hi = mid - 1` (9 FAIL плюс `heap-buffer-overflow` под AddressSanitizer), проверка только краёв (10 FAIL), всегда середина (34 FAIL), линейный проход (таймаут на возрастающем массиве из миллиона).
- Сравнение через разность `nums[mid + 1] - nums[mid]` при обычной сборке с `-O2` проходит, потому что компилятор переписывает его в `>`, но под `-fsanitize=undefined` даёт `signed integer overflow` на тестах с крайними значениями. Запускайте решение и с санитайзерами.

Пришлите решение, и я его разберу.

---

work: 22:10 - 

---

- {1, 2, 1, 3, 5, 6, 4} => lo = 0, hi 6  => lo + (hi - lo) / 2 = 3
- {1, 2, 1, [3], 5, 6, 4} => 1 < 3 & 5 > 3 => not peakl
- ...
- ...
- 1, 2, 1, {3, [5], 6, 4} => lo = 3, hi = 6 => 3 + (6 - 3) / 2 = 4

---

```cpp
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <cstddef>

namespace {
	int find_peak(const std::vector<int>& nums) {
		const std::size_t size{nums.size()};
		const std::size_t lo{0};
		const std::size_t hi{size - 1};
		
		const auto check_left = [&nums](const std::size_t idx) -> bool {
			if (idx == 0) return true;
			return  nums[idx - 1] < nums[idx];
		};
		const auto check_right = [&nums, &size](const std::size_t idx) -> bool {
			return idx == size - 1 || nums[idx + 1]  < nums[idx];
		};
		
		while (true) {
			const auto maybe_peak_idx{lo + (hi - lo) / 2};
			if (check_left(maybe_peak_idx) && check_right(maybe_peak_idx)) {
				return static_cast<int>(maybe_peak_idx);
			}
			
			if (nums[maybe_peak_idx + 1] < nums[maybe_peak_idx]) {
				lo = mid + 1;
			} else {
				hi = mid;
			}
		}
	
		return 0;
	}

	// any peak is a correct answer: both neighbours (if they exist) are smaller
	bool is_peak(const std::vector<int>& nums, int index) {
		const int size{static_cast<int>(nums.size())};
		if (index < 0 || index >= size) return false;
		const int value{nums[static_cast<std::size_t>(index)]};
		if (index > 0 && nums[static_cast<std::size_t>(index - 1)] > value) return false;
		if (index + 1 < size && nums[static_cast<std::size_t>(index + 1)] > value) return false;
		return true;
	}

	void test_peak(const std::string& label,
				   const std::vector<int>& nums) {
		const auto result{find_peak(nums)};

		std::cout << std::format("{}: {}\n",
			label,
			is_peak(nums, result) ? "OK" : "FAIL") << std::flush;
	}

	// the same array queried many times: a linear scan cannot pass this
	void test_peak_many(const std::string& label,
						const std::vector<int>& nums,
						int repeats) {
		bool ok{true};
		for (int i{0}; i < repeats; ++i) {
			if (!is_peak(nums, find_peak(nums))) {
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
	test_peak("description example 1",
		{1, 2, 3, 1});
	test_peak("description example 2",
		{1, 2, 1, 3, 5, 6, 4});
	test_peak("single element",
		{7});
	test_peak("single element, minimum int",
		{-2147483648});
	test_peak("single element, maximum int",
		{2147483647});
	test_peak("two elements, rising",
		{1, 2});
	test_peak("two elements, falling",
		{2, 1});
	test_peak("two elements, extreme, falling",
		{2147483647, -2147483648});
	test_peak("two elements, extreme, rising",
		{-2147483648, 2147483647});
	test_peak("three elements, peak in the middle",
		{1, 3, 2});
	test_peak("three elements, valley in the middle",
		{3, 1, 2});
	test_peak("three elements, rising",
		{1, 2, 3});
	test_peak("three elements, falling",
		{3, 2, 1});
	test_peak("strictly increasing",
		{1, 2, 3, 4, 5, 6, 7, 8});
	test_peak("strictly decreasing",
		{8, 7, 6, 5, 4, 3, 2, 1});
	test_peak("mountain, peak in the middle",
		{1, 2, 3, 4, 5, 4, 3, 2, 1});
	test_peak("mountain, peak at the second position",
		{1, 9, 5, 4, 3, 2, 1});
	test_peak("mountain, peak at the second to last position",
		{1, 2, 3, 4, 5, 9, 1});
	test_peak("valley, two peaks at the ends",
		{9, 5, 1, 2, 6});
	test_peak("valley, deep",
		{10, 8, 6, 4, 2, 1, 3, 5, 7, 9});
	test_peak("zigzag starting up",
		{1, 5, 2, 6, 3, 7, 4});
	test_peak("zigzag starting down",
		{5, 1, 6, 2, 7, 3, 8});
	test_peak("zigzag, even length",
		{1, 5, 2, 6, 3, 7});
	test_peak("two peaks, the left one is higher",
		{1, 9, 2, 3, 4, 5, 4});
	test_peak("two peaks, the right one is higher",
		{1, 4, 2, 3, 4, 9, 8});
	test_peak("three peaks",
		{1, 3, 1, 5, 1, 7, 1});
	test_peak("plateau-free staircase up then down",
		{1, 2, 4, 7, 11, 16, 10, 6, 3, 1});
	test_peak("negative values, rising",
		{-9, -7, -5, -3, -1});
	test_peak("negative values, falling",
		{-1, -3, -5, -7, -9});
	test_peak("negative values, mountain",
		{-9, -5, -1, -4, -8});
	test_peak("mixed signs, zigzag",
		{-3, 4, -5, 6, -7, 8, -9});
	test_peak("extreme values alternate",
		{-2147483648, 2147483647, -2147483648, 2147483647, -2147483648});
	test_peak("extreme values alternate, starting high",
		{2147483647, -2147483648, 2147483647, -2147483648, 2147483647});
	test_peak("extreme values, rising to maximum",
		{-2147483648, 0, 2147483647});
	test_peak("extreme values, falling from maximum",
		{2147483647, 0, -2147483648});
	test_peak("extreme values, differences overflow int",
		{2147483647, -2147483648, 2147483647, -2147483648});
	test_peak("extreme values, differences overflow int, rising",
		{-2147483648, 2147483647, -2147483648, 2147483647});
	test_peak("long rise then one drop",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 19});
	test_peak("one rise then long fall",
		{1, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1});
	test_peak("random 1, n=1",
		{20});
	test_peak("random 2, n=4",
		{12, -8, 6, 11});
	test_peak("random 3, n=13",
		{19, -2, 8, 9, -19, -9, -13, 6, 0, 15, -16, 9, 16});
	test_peak("random 4, n=1",
		{-3});
	test_peak("random 5, n=2",
		{-11, 6});
	test_peak("random 6, n=8",
		{-18, -20, -7, -9, 12, -20, 19, -13});
	test_peak("random 7, n=8",
		{12, -4, -6, 15, -3, -7, 5, -5});
	test_peak("random 8, n=14",
		{-4, 5, -8, 18, 2, 20, 19, -5, -19, -4, -19, 16, -6, -20});

	{
		// n = 1000000, strictly increasing: the only peak is the last element
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i;
		test_peak_many("n = 1000000, strictly increasing, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, strictly decreasing: the only peak is the first element
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = 1000000 - i;
		test_peak_many("n = 1000000, strictly decreasing, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, mountain with the peak at 777777
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) {
			nums[static_cast<std::size_t>(i)] = i <= 777777 ? i : 1555554 - i;
		}
		test_peak_many("n = 1000000, mountain, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, valley: two peaks at the ends
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) {
			nums[static_cast<std::size_t>(i)] = i <= 500000 ? 500000 - i : i - 500000;
		}
		test_peak_many("n = 1000000, valley, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, zigzag
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = (i % 2 == 0) ? i : -i;
		test_peak_many("n = 1000000, zigzag, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, extreme values alternate: the differences overflow int
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) {
			nums[static_cast<std::size_t>(i)] = (i % 2 == 0) ? 2147483647 : -2147483647 - 1;
		}
		test_peak_many("n = 1000000, extreme alternation, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, extreme values rise in two big steps
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) {
			nums[static_cast<std::size_t>(i)] = i < 500000 ? -2147483647 - 1 + i : 2147483647 - (999999 - i);
		}
		test_peak_many("n = 1000000, extreme steps, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, pseudo-random values with different neighbours
		std::vector<int> nums(1000000);
		unsigned state{162};
		int previous{0};
		for (auto& x : nums) {
			do {
				state = state * 1103515245u + 12345u;
				x = static_cast<int>(state);
			} while (x == previous);
			previous = x;
		}
		test_peak_many("n = 1000000, random, 100000 queries", nums, 100000);
	}
	{
		// n = 1000000, sawtooth with period 1000: many peaks
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i % 1000;
		test_peak_many("n = 1000000, sawtooth, 100000 queries", nums, 100000);
	}

	return 0;
}
```
