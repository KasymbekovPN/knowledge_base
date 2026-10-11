## Задача: Contains Duplicate (LeetCode 217, easy)

**Условие.** Дан массив целых чисел `nums`. Нужно вернуть `true`, если какое-нибудь значение встречается в массиве **не менее двух раз**, и `false`, если все элементы различны.

**Сигнатура:**

```cpp
bool containsDuplicate(std::vector<int>& nums);
```

**Ограничения:**

- `1 <= nums.size() <= 10^5`
- `-10^9 <= nums[i] <= 10^9`

**Что обычно спрашивает интервьюер:**

1. Решение в лоб и его сложность.
2. Решение через сортировку. Какая сложность по времени и по памяти? Что делать, если массив нельзя менять?
3. Решение через хеш-таблицу. Сложность? Что выгоднее: заполнить всё и сравнить размеры или выходить при первом повторе?
4. `std::unordered_set` или `std::set`: что быстрее в среднем, что в худшем случае, что по памяти?
5. Можно ли заменить множество на `std::bitset` или массив флагов, учитывая диапазон значений?
6. Если сортируете сами, как правильно написать компаратор? Что случится, если сравнивать через вычитание `a - b`?
7. Что изменится, если данные приходят потоком и не помещаются в память?

**Целевая сложность:** O(n) по времени в среднем и O(n) по памяти (хеш-таблица) либо O(n log n) по времени и O(1) дополнительной памяти (сортировка на месте).

## Шаблон `main`

Решения в шаблоне нет, рабочий метод пустой и возвращает `false`. Из-за этого 13 тестов с ожидаемым `false` проходят случайно, а все 19 тестов с ожидаемым `true` падают. Это нормально для заглушки. Ожидаемые значения проверены независимо: эталонное решение проходит все 32 теста под `-fsanitize=address,undefined`, и шаблон собирается без предупреждений.

```cpp
namespace {
	using Nums = std::vector<int>;

	bool get_contains_duplicate([[maybe_unused]] const Nums& nums) {
		return false;
	}

	void test_contains_duplicate(const std::string& label,
								 const Nums& nums,
								 const bool expected) {
		const auto result{get_contains_duplicate(nums)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_contains_duplicate("description example 1",
		{1, 2, 3, 1},
		true);
	test_contains_duplicate("description example 2",
		{1, 2, 3, 4},
		false);
	test_contains_duplicate("description example 3",
		{1, 1, 1, 3, 3, 4, 3, 2, 4, 2},
		true);
	test_contains_duplicate("single element",
		{5},
		false);
	test_contains_duplicate("two equal",
		{7, 7},
		true);
	test_contains_duplicate("two different",
		{7, 8},
		false);
	test_contains_duplicate("adjacent duplicates",
		{1, 5, 5, 9},
		true);
	test_contains_duplicate("duplicate at the ends",
		{4, 1, 2, 3, 4},
		true);
	test_contains_duplicate("duplicate far apart",
		{9, 1, 2, 3, 4, 5, 6, 7, 8, 9},
		true);
	test_contains_duplicate("all equal",
		{3, 3, 3, 3, 3},
		true);
	test_contains_duplicate("zeros",
		{0, 1, 0},
		true);
	test_contains_duplicate("single zero",
		{0, 1, 2},
		false);
	test_contains_duplicate("negative duplicates",
		{-5, 3, -5},
		true);
	test_contains_duplicate("negatives, all distinct",
		{-5, -4, -3, -2, -1},
		false);
	test_contains_duplicate("sign differs, not a duplicate",
		{-3, 3, -2, 2, 0},
		false);
	test_contains_duplicate("max value duplicates",
		{1000000000, 5, 1000000000},
		true);
	test_contains_duplicate("min value duplicates",
		{-1000000000, 5, -1000000000},
		true);
	test_contains_duplicate("extreme neighbours, distinct",
		{1000000000, -1000000000, 999999999, -999999999},
		false);
	test_contains_duplicate("extreme neighbours, with duplicate",
		{1000000000, -1000000000, 999999999, -1000000000},
		true);
	test_contains_duplicate("sorted distinct",
		{1, 2, 3, 4, 5, 6, 7, 8},
		false);
	test_contains_duplicate("reverse sorted with duplicate",
		{9, 8, 7, 6, 5, 5, 3, 2, 1},
		true);
	test_contains_duplicate("random distinct 1, n=12",
		{-27, -40, -53, -36, 78, -37, -69, -13, -62, -32, 91, -18},
		false);
	test_contains_duplicate("random distinct 2, n=12",
		{68, 96, 8, 40, 49, 83, -46, -77, -79, -27, -80, -88},
		false);
	test_contains_duplicate("random distinct 3, n=12",
		{43, -10, -82, -36, -19, -26, 10, -59, 22, 39, 0, 52},
		false);
	test_contains_duplicate("random with one duplicate 1, n=12",
		{-26, 70, -22, -1, 47, -85, 72, -22, 4, 69, -100, -13},
		true);
	test_contains_duplicate("random with one duplicate 2, n=12",
		{86, -92, 55, -20, 4, 21, -8, 23, 83, 72, -3, 72},
		true);
	test_contains_duplicate("random with one duplicate 3, n=12",
		{96, 79, 60, -28, -90, 54, -52, 90, 88, 100, -25, 54},
		true);
	{
		// n = 100000, all distinct
		Nums nums(100000);
		std::iota(nums.begin(), nums.end(), -50000);
		test_contains_duplicate("n = 100000, all distinct", nums, false);
	}
	{
		// n = 100000, all distinct, duplicate of the first element at the very end
		Nums nums(100000);
		std::iota(nums.begin(), nums.end(), -50000);
		nums.back() = nums.front();
		test_contains_duplicate("n = 100000, duplicate at the extremes", nums, true);
	}
	{
		// n = 100000, all distinct, duplicate pair in the middle
		Nums nums(100000);
		std::iota(nums.begin(), nums.end(), -50000);
		nums[70000] = nums[30000];
		test_contains_duplicate("n = 100000, duplicate in the middle", nums, true);
	}
	{
		// n = 100000, all equal
		test_contains_duplicate("n = 100000, all equal", Nums(100000, 42), true);
	}
	{
		// n = 100000, random values in [-1e9, 1e9], all distinct (seed checked)
		std::mt19937 rng{6};
		Nums nums(100000);
		for (auto& x : nums) x = static_cast<int>(rng() % 2000000001u) - 1000000000;
		test_contains_duplicate("n = 100000, random values", nums, false);
	}

	return 0;
}
```

Что покрывают тесты:

- **ответ `false`:** пять случаев, где ничего не повторяется, включая `n = 1`, значения `±` одинакового модуля и крайние значения `±10^9`. Это ловит решения с ложными срабатываниями (например, сравнение по модулю);
- **расположение дубликата:** рядом, на краях, далеко друг от друга, в середине. Это проверяет, что решение не пропускает пару из-за ограничения диапазона цикла;
- **границы значений:** `10^9` и `-10^9` как дубликаты и как соседи без дубликата. Компаратор вида `a - b` на таких значениях переполняется;
- **n = 100 000:** все различны (худший случай для хеш-таблицы, нужно просмотреть всё), дубликат на самых краях, дубликат в середине, все равны, случайные значения без повторов (проверено по seed). Квадратичное решение здесь заметно зависает.

Пришлите решение, и я разберу его с секцией `to error log`.

---

работа: 13:32 - 13:48

---

- {1, 2, 3, 1} => {1: 1}
- {1, 2, 3, 1} => {1: 1, 2: 1}
- {1, 2, 3, 1} => {1: 1, 2: 1, 3: 1}
- {1, 2, 3, 1} => {1: 2, 2: 1, 3: 1} ==> true

---

- {1, 2, 3, 1} => {1, 1, 2, 3}
- {1, [1], 2, 3} => prev == curr => true

---


```cpp
#include <random>
#include <numeric>
#include <iostream>
#include <format>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>

namespace {
	using Nums = std::vector<int>;

	[[nodiscard]] bool get_contains_duplicate_0(const Nums& nums) {
		std::unordered_set<int> seen;
		seen.reserve(size);
		
		for (const auto current_value: nums) {
			if (seen.contains(current_value)) return true;
			
			seen.insert(current_value);
		}
	
		return false;
	}
	
	[[nodiscard]] bool get_contains_duplicate_1(Nums nums) {
		std::ranges::sort(nums);
		return std::ranges::adjacent_find(nums) != nums.end();
	}

	void test_contains_duplicate(const std::string& label,
								 const Nums& nums,
								 const bool expected) {
		const auto result_1{get_contains_duplicate_1(nums)};
		const auto result_0{get_contains_duplicate_0(nums)};
		
		const auto get_res_as_str = [&expected](const bool res) {
			return res == expected ? "OK" : "FAIL";
		};
		std::cout << std::format("{}: 0 {}, 1: {}\n",
			label,
			get_res_as_str(result_0),
			get_res_as_str(result_1)) << std::flush;
	}
}

int main() {
	test_contains_duplicate("description example 1",
		{1, 2, 3, 1},
		true);
	test_contains_duplicate("description example 2",
		{1, 2, 3, 4},
		false);
	test_contains_duplicate("description example 3",
		{1, 1, 1, 3, 3, 4, 3, 2, 4, 2},
		true);
	test_contains_duplicate("single element",
		{5},
		false);
	test_contains_duplicate("two equal",
		{7, 7},
		true);
	test_contains_duplicate("two different",
		{7, 8},
		false);
	test_contains_duplicate("adjacent duplicates",
		{1, 5, 5, 9},
		true);
	test_contains_duplicate("duplicate at the ends",
		{4, 1, 2, 3, 4},
		true);
	test_contains_duplicate("duplicate far apart",
		{9, 1, 2, 3, 4, 5, 6, 7, 8, 9},
		true);
	test_contains_duplicate("all equal",
		{3, 3, 3, 3, 3},
		true);
	test_contains_duplicate("zeros",
		{0, 1, 0},
		true);
	test_contains_duplicate("single zero",
		{0, 1, 2},
		false);
	test_contains_duplicate("negative duplicates",
		{-5, 3, -5},
		true);
	test_contains_duplicate("negatives, all distinct",
		{-5, -4, -3, -2, -1},
		false);
	test_contains_duplicate("sign differs, not a duplicate",
		{-3, 3, -2, 2, 0},
		false);
	test_contains_duplicate("max value duplicates",
		{1000000000, 5, 1000000000},
		true);
	test_contains_duplicate("min value duplicates",
		{-1000000000, 5, -1000000000},
		true);
	test_contains_duplicate("extreme neighbours, distinct",
		{1000000000, -1000000000, 999999999, -999999999},
		false);
	test_contains_duplicate("extreme neighbours, with duplicate",
		{1000000000, -1000000000, 999999999, -1000000000},
		true);
	test_contains_duplicate("sorted distinct",
		{1, 2, 3, 4, 5, 6, 7, 8},
		false);
	test_contains_duplicate("reverse sorted with duplicate",
		{9, 8, 7, 6, 5, 5, 3, 2, 1},
		true);
	test_contains_duplicate("random distinct 1, n=12",
		{-27, -40, -53, -36, 78, -37, -69, -13, -62, -32, 91, -18},
		false);
	test_contains_duplicate("random distinct 2, n=12",
		{68, 96, 8, 40, 49, 83, -46, -77, -79, -27, -80, -88},
		false);
	test_contains_duplicate("random distinct 3, n=12",
		{43, -10, -82, -36, -19, -26, 10, -59, 22, 39, 0, 52},
		false);
	test_contains_duplicate("random with one duplicate 1, n=12",
		{-26, 70, -22, -1, 47, -85, 72, -22, 4, 69, -100, -13},
		true);
	test_contains_duplicate("random with one duplicate 2, n=12",
		{86, -92, 55, -20, 4, 21, -8, 23, 83, 72, -3, 72},
		true);
	test_contains_duplicate("random with one duplicate 3, n=12",
		{96, 79, 60, -28, -90, 54, -52, 90, 88, 100, -25, 54},
		true);
	{
		// n = 100000, all distinct
		Nums nums(100000);
		std::iota(nums.begin(), nums.end(), -50000);
		test_contains_duplicate("n = 100000, all distinct", nums, false);
	}
	{
		// n = 100000, all distinct, duplicate of the first element at the very end
		Nums nums(100000);
		std::iota(nums.begin(), nums.end(), -50000);
		nums.back() = nums.front();
		test_contains_duplicate("n = 100000, duplicate at the extremes", nums, true);
	}
	{
		// n = 100000, all distinct, duplicate pair in the middle
		Nums nums(100000);
		std::iota(nums.begin(), nums.end(), -50000);
		nums[70000] = nums[30000];
		test_contains_duplicate("n = 100000, duplicate in the middle", nums, true);
	}
	{
		// n = 100000, all equal
		test_contains_duplicate("n = 100000, all equal", Nums(100000, 42), true);
	}
	{
		// n = 100000, random values in [-1e9, 1e9], all distinct (seed checked)
		std::mt19937 rng{6};
		Nums nums(100000);
		for (auto& x : nums) x = static_cast<int>(rng() % 2000000001u) - 1000000000;
		test_contains_duplicate("n = 100000, random values", nums, false);
	}

	return 0;
}
```