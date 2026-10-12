## 560. Subarray Sum Equals K (Medium)

Дан целочисленный массив `nums` и целое число `k`. Нужно вернуть **количество непрерывных подмассивов**, сумма элементов которых равна `k`.

Подмассив — непрерывная непустая последовательность элементов массива. Подмассивы, совпадающие по значениям, но расположенные в разных местах, считаются разными.

**Примеры**

- `nums = [1,1,1]`, `k = 2` → `2`
- `nums = [1,2,3]`, `k = 3` → `2`

**Сигнатура**

```cpp
int subarray_sum(const std::vector<int>& nums, int k);
```

**Ограничения**

- `1 <= nums.length <= 2 * 10^4`
- `-1000 <= nums[i] <= 1000`
- `-10^7 <= k <= 10^7`

**Вопросы интервьюера**

1. Какое решение «в лоб» и какая у него сложность? Влезает ли оно в ограничения?
2. Почему здесь не работает метод скользящего окна (два указателя)?
3. Что такое префиксные суммы и как они сводят задачу к «найти пару» (как в Two Sum)?
4. Что хранить в хеш-таблице: ключ, значение? Почему нельзя ограничиться множеством?
5. Зачем нужен «пустой префикс» с суммой 0 и в какой момент обновлять таблицу: до или после проверки?
6. Как вы убедитесь, что `int` не переполнится: какова максимальная префиксная сумма и максимальный ответ?
7. Как изменится решение для `unordered_map`, если опасаться адверсарных тестов на коллизии?

**Целевая сложность:** O(n) по времени в среднем, O(n) по памяти.

**Шаблон**

```cpp
namespace {
	int subarray_sum([[maybe_unused]] const std::vector<int>& nums,
					 [[maybe_unused]] const int k) {
		return 0;
	}

	void test_subarray_sum(const std::string& label,
						   const std::vector<int>& nums,
						   const int k,
						   const int expected) {
		const auto result{subarray_sum(nums, k)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_subarray_sum("description example 1",
		{1, 1, 1},
		2,
		2);
	test_subarray_sum("description example 2",
		{1, 2, 3},
		3,
		2);
	test_subarray_sum("single element, equal",
		{5},
		5,
		1);
	test_subarray_sum("single element, different",
		{5},
		4,
		0);
	test_subarray_sum("single negative element, equal",
		{-5},
		-5,
		1);
	test_subarray_sum("single zero, k = 0",
		{0},
		0,
		1);
	test_subarray_sum("whole array is the only answer",
		{1, 2, 3, 4},
		10,
		1);
	test_subarray_sum("prefix from the start (empty prefix is needed)",
		{3, 4, 7, 2},
		7,
		2);
	test_subarray_sum("no subarray",
		{1, 2, 3},
		100,
		0);
	test_subarray_sum("k is negative",
		{-1, -1, 1},
		-2,
		1);
	test_subarray_sum("negatives break the sliding window",
		{1, -1, 0},
		0,
		3);
	test_subarray_sum("negatives, several answers",
		{-1, -1, 1},
		0,
		1);
	test_subarray_sum("zeros with k = 0",
		{0, 0, 0},
		0,
		6);
	test_subarray_sum("zeros with k != 0",
		{0, 0, 0},
		1,
		0);
	test_subarray_sum("zeros around the answer",
		{0, 0, 1, 0, 0},
		1,
		9);
	test_subarray_sum("overlapping answers",
		{1, 1, 1, 1},
		2,
		3);
	test_subarray_sum("sum goes down and up again",
		{3, -3, 3, -3, 3},
		3,
		6);
	test_subarray_sum("alternating signs, k = 0",
		{1, -1, 1, -1},
		0,
		4);
	test_subarray_sum("all negative, k = sum of two",
		{-2, -3, -4, -5},
		-7,
		1);
	test_subarray_sum("same prefix sum repeats many times",
		{2, -2, 2, -2, 2, -2},
		0,
		9);
	test_subarray_sum("max values",
		{1000, 1000, 1000},
		2000,
		2);
	test_subarray_sum("min values",
		{-1000, -1000, -1000},
		-2000,
		2);
	test_subarray_sum("extreme k, no answer",
		{1000, -1000, 1000},
		10000000,
		0);
	test_subarray_sum("extreme negative k, no answer",
		{1000, -1000, 1000},
		-10000000,
		0);
	test_subarray_sum("k equals the maximal subarray sum",
		{1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
		20000,
		1);
	test_subarray_sum("random 1, n=9, k=-4",
		{3, 3, -4, 2, 1, -1, -1, -3, 2},
		-4,
		4);
	test_subarray_sum("random 2, n=21, k=-1",
		{-1, -2, 3, -2, 3, 0, -3, -3, 0, -2, 0, 3, -1, -3, -2, 3, -2, 3, 1, -1, 2},
		-1,
		17);
	test_subarray_sum("random 3, n=11, k=2",
		{-3, -2, -1, 4, 0, -1, -3, -4, 4, -3, -2},
		2,
		1);
	test_subarray_sum("random 4, n=8, k=3",
		{-2, 3, 2, 3, 2, 0, -3, -2},
		3,
		4);
	test_subarray_sum("random wide values 1, n=22, k=612",
		{-637, -803, 831, 572, -900, 538, -993, -649, 965, 811, 612, 882, 616, -643, 301, -674, -678, -587, 528, 329, 394, 520},
		612,
		1);
	test_subarray_sum("random wide values 2, n=28, k=380",
		{-442, 932, -206, -414, -415, -582, -62, 695, 260, -93, -344, 605, -587, 544, -890, -473, 888, -508, -931, 863, 908, -57, -289, -438, -50, -648, 138, -694},
		380,
		1);

	{
		// all zeros, every subarray sums to 0: n(n+1)/2
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = 0;
		test_subarray_sum("n = 20000, all zeros, k = 0",
			nums,
			0,
			200010000);
	}
	{
		// all zeros, nobody sums to 1
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = 0;
		test_subarray_sum("n = 20000, all zeros, k = 1",
			nums,
			1,
			0);
	}
	{
		// all ones: subarrays of length 10000
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = 1;
		test_subarray_sum("n = 20000, all ones, k = 10000",
			nums,
			10000,
			10001);
	}
	{
		// 1, -1, 1, -1, ...: even length subarrays
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = i % 2 == 0 ? 1 : -1;
		test_subarray_sum("n = 20000, 1 and -1 alternate, k = 0",
			nums,
			0,
			100000000);
	}
	{
		// 1, -1, 1, -1, ...: odd length subarrays that start at an even index
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = i % 2 == 0 ? 1 : -1;
		test_subarray_sum("n = 20000, 1 and -1 alternate, k = 1",
			nums,
			1,
			50005000);
	}
	{
		// pseudo-random values in [-1000, 1000], k = 0
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = static_cast<int>(i * 37 % 2001) - 1000;
		test_subarray_sum("n = 20000, pseudo-random values, k = 0",
			nums,
			0,
			240654);
	}
	{
		// pseudo-random values in [-1000, 1000], k = 1000
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = static_cast<int>(i * 37 % 2001) - 1000;
		test_subarray_sum("n = 20000, pseudo-random values, k = 1000",
			nums,
			1000,
			10220);
	}
	{
		// all 1000: total sum is 20000000, k = 10^7 is half of it
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = 1000;
		test_subarray_sum("n = 20000, all 1000, k = 10000000",
			nums,
			10000000,
			10001);
	}
	{
		// all -1000, k = -10^7
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = -1000;
		test_subarray_sum("n = 20000, all -1000, k = -10000000",
			nums,
			-10000000,
			10001);
	}

	return 0;
}
```

Тесты не отличат O(n²) от O(n) по времени: при n = 20000 квадратичное решение укладывается примерно в секунду. Сложность проверит интервьюер, поэтому её нужно обосновать вслух. Пришлите свою попытку, и я её разберу.

---

work : 14:22 - 

---

- -2, -3, -4, -5 | expected_sum = -7
- [-2], -3, -4, -5 | expected_sum = -7, current_sum = -2
- [-2, -3], -4, -5 | expected_sum = -7, current_sum = -5
- [-2, -3, -4], -5 | expected_sum = -7, current_sum = -5
- [-2, -3, -4], -5 | expected_sum = -7, current_sum = -9
- -2, [-3, -4], -5 | expected_sum = -7, current_sum = -7 (+)

- -2, -3, -4, -5 | pref(0)
- -2, -3, -4, -5 | pref(0, -2)
- -2, -3, -4, -5 | pref(0, -2, -5)
- -2, -3, -4, -5 | pref(0, -2, -5, -9)
- -2, -3, -4, -5 | pref(0, -2, -5, -9, -14)

---

- 1, -1, 1, 1, -1 | expected_sum = 2 
- [1], -1, 1, 1, -1 | expected_sum = 2 , sum = 1
- [1, -1], 1, 1, -1 | expected_sum = 2 , sum = 0
- [1, -1, 1], 1, -1 | expected_sum = 2 , sum = 1
- [1, -1, [1, 1]], -1 | expected_sum = 2 , sum = 2 (-)

---

- 3, -3, 3, -3, 3 | expected_sum=3, pref_sum[]
- [3], -3, 3, -3, 3 | expected_sum=3, pref_sum[0]
- 3, [-3], 3, -3, 3 | expected_sum=3, pref_sum[0, 3]
- 3, -3, [3], -3, 3 | expected_sum=3, pref_sum[0, 3, 0]
- 3, -3, 3, [-3], 3 | expected_sum=3, pref_sum[0, 3, 0, 3]
- 3, -3, 3, -3, [3] | expected_sum=3, pref_sum[0, 3, 0, 3, 0]

- [0], 3, 0, 3, 0 | {0 : {0}} | {0 : 1
- 0, [3], 0, 3, 0 | {0 : {0}, 3: {1}} | {0 : 1, 3: 1
- 0, 3, [0], 3, 0 | {0 : {0, 2}, 3: {1}} |  {0 : 2, 3: 1}
- 0, 3, 0, [3], 0 | {0 : {0, 2}, 3: {1, 3}} | {0 : 2, 3: 2}
- 0, 3, 0, 3, [0] | {0 : {0, 2, 3}, 3: {1, 3}} | {0 : 3, 3: 2}

- [3], -3, 3, -3, 3 | (3)
- 3, [-3], 3, -3, 3 | (3, 0)
- 3, -3, [3], -3, 3 | (3, 0, 3)
- 3, -3, 3, [-3], 3 | (3, 0, 3, 0)
- 3, -3, 3, -3, [3] | (3, 0, 3, 0, 3)

---




---

```cpp
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef>

namespace {
	int subarray_sum(const std::vector<int>& nums,
					 const int k) {

		const std::size_t size{nums.size()};
		if (size == 0) return 0;
		
		int prefix{0};
		std::unordered_map<int, int> prefix_idx_by_value;
		prefix_idx_by_value[0] = 1;

		int total{0};
		for (std::size_t i{0}; i < size; ++i) {
			prefix += nums[i];

			auto it{prefix_idx_by_value.find(prefix - k)};
			if (it != prefix_idx_by_value.end()) {
				total += it->second;
			}
			
			++prefix_idx_by_value[prefix];
			
		}

		return total;
	}

	void test_subarray_sum(const std::string& label,
						   const std::vector<int>& nums,
						   const int k,
						   const int expected) {
		const auto result{subarray_sum(nums, k)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_subarray_sum("description example 1",
		{1, 1, 1},
		2,
		2);
	test_subarray_sum("description example 2",
		{1, 2, 3},
		3,
		2);
	test_subarray_sum("single element, equal",
		{5},
		5,
		1);
	test_subarray_sum("single element, different",
		{5},
		4,
		0);
	test_subarray_sum("single negative element, equal",
		{-5},
		-5,
		1);
	test_subarray_sum("single zero, k = 0",
		{0},
		0,
		1);
	test_subarray_sum("whole array is the only answer",
		{1, 2, 3, 4},
		10,
		1);
	test_subarray_sum("prefix from the start (empty prefix is needed)",
		{3, 4, 7, 2},
		7,
		2);
	test_subarray_sum("no subarray",
		{1, 2, 3},
		100,
		0);
	test_subarray_sum("k is negative",
		{-1, -1, 1},
		-2,
		1);
	test_subarray_sum("negatives break the sliding window",
		{1, -1, 0},
		0,
		3);
	test_subarray_sum("negatives, several answers",
		{-1, -1, 1},
		0,
		1);
	test_subarray_sum("zeros with k = 0",
		{0, 0, 0},
		0,
		6);
	test_subarray_sum("zeros with k != 0",
		{0, 0, 0},
		1,
		0);
	test_subarray_sum("zeros around the answer",
		{0, 0, 1, 0, 0},
		1,
		9);
	test_subarray_sum("overlapping answers",
		{1, 1, 1, 1},
		2,
		3);
	test_subarray_sum("sum goes down and up again",
		{3, -3, 3, -3, 3},
		3,
		6);
	test_subarray_sum("alternating signs, k = 0",
		{1, -1, 1, -1},
		0,
		4);
	test_subarray_sum("all negative, k = sum of two",
		{-2, -3, -4, -5},
		-7,
		1);
	test_subarray_sum("same prefix sum repeats many times",
		{2, -2, 2, -2, 2, -2},
		0,
		9);
	test_subarray_sum("max values",
		{1000, 1000, 1000},
		2000,
		2);
	test_subarray_sum("min values",
		{-1000, -1000, -1000},
		-2000,
		2);
	test_subarray_sum("extreme k, no answer",
		{1000, -1000, 1000},
		10000000,
		0);
	test_subarray_sum("extreme negative k, no answer",
		{1000, -1000, 1000},
		-10000000,
		0);
	test_subarray_sum("k equals the maximal subarray sum",
		{1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
		20000,
		1);
	test_subarray_sum("random 1, n=9, k=-4",
		{3, 3, -4, 2, 1, -1, -1, -3, 2},
		-4,
		4);
	test_subarray_sum("random 2, n=21, k=-1",
		{-1, -2, 3, -2, 3, 0, -3, -3, 0, -2, 0, 3, -1, -3, -2, 3, -2, 3, 1, -1, 2},
		-1,
		17);
	test_subarray_sum("random 3, n=11, k=2",
		{-3, -2, -1, 4, 0, -1, -3, -4, 4, -3, -2},
		2,
		1);
	test_subarray_sum("random 4, n=8, k=3",
		{-2, 3, 2, 3, 2, 0, -3, -2},
		3,
		4);
	test_subarray_sum("random wide values 1, n=22, k=612",
		{-637, -803, 831, 572, -900, 538, -993, -649, 965, 811, 612, 882, 616, -643, 301, -674, -678, -587, 528, 329, 394, 520},
		612,
		1);
	test_subarray_sum("random wide values 2, n=28, k=380",
		{-442, 932, -206, -414, -415, -582, -62, 695, 260, -93, -344, 605, -587, 544, -890, -473, 888, -508, -931, 863, 908, -57, -289, -438, -50, -648, 138, -694},
		380,
		1);

	{
		// all zeros, every subarray sums to 0: n(n+1)/2
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = 0;
		test_subarray_sum("n = 20000, all zeros, k = 0",
			nums,
			0,
			200010000);
	}
	{
		// all zeros, nobody sums to 1
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = 0;
		test_subarray_sum("n = 20000, all zeros, k = 1",
			nums,
			1,
			0);
	}
	{
		// all ones: subarrays of length 10000
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = 1;
		test_subarray_sum("n = 20000, all ones, k = 10000",
			nums,
			10000,
			10001);
	}
	{
		// 1, -1, 1, -1, ...: even length subarrays
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = i % 2 == 0 ? 1 : -1;
		test_subarray_sum("n = 20000, 1 and -1 alternate, k = 0",
			nums,
			0,
			100000000);
	}
	{
		// 1, -1, 1, -1, ...: odd length subarrays that start at an even index
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = i % 2 == 0 ? 1 : -1;
		test_subarray_sum("n = 20000, 1 and -1 alternate, k = 1",
			nums,
			1,
			50005000);
	}
	{
		// pseudo-random values in [-1000, 1000], k = 0
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = static_cast<int>(i * 37 % 2001) - 1000;
		test_subarray_sum("n = 20000, pseudo-random values, k = 0",
			nums,
			0,
			240654);
	}
	{
		// pseudo-random values in [-1000, 1000], k = 1000
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = static_cast<int>(i * 37 % 2001) - 1000;
		test_subarray_sum("n = 20000, pseudo-random values, k = 1000",
			nums,
			1000,
			10220);
	}
	{
		// all 1000: total sum is 20000000, k = 10^7 is half of it
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = 1000;
		test_subarray_sum("n = 20000, all 1000, k = 10000000",
			nums,
			10000000,
			10001);
	}
	{
		// all -1000, k = -10^7
		std::vector<int> nums(20000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = -1000;
		test_subarray_sum("n = 20000, all -1000, k = -10000000",
			nums,
			-10000000,
			10001);
	}

	return 0;
}
```
