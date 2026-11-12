## Блок 4 (сортировки), задача 3: 215. Kth Largest Element in an Array

**Условие.** Дан массив целых `nums` и число `k`. Нужно вернуть `k`-й по величине элемент массива. Это `k`-й элемент в порядке убывания с учётом повторов, а не `k`-й различный. Например, в `{5, 5, 4}` второй по величине — это 5.

Функция может переставлять элементы `nums`. Для окончательного решения нельзя использовать `std::sort`, `std::nth_element` и `std::partial_sort` (в том числе `std::ranges`-версии).

**Сигнатура.**

```cpp
int find_kth_largest(std::vector<int>& nums, int k);
```

**Ограничения.**

- 1 ≤ k ≤ n ≤ 10⁶
- −10⁹ ≤ `nums[i]` ≤ 10⁹

**Целевая сложность.** В среднем O(n) по времени, O(1) дополнительной памяти.

**Вопросы интервьюера** (вслух, в процессе):

1. Какое самое простое решение? Что с временем и памятью?
2. Как улучшить, если держать только `k` лучших элементов? Когда это лучше сортировки, а когда нет?
3. Нужно ли упорядочивать весь массив, чтобы найти один элемент? Какая часть информации на самом деле нужна?
4. Как устроен выбор через разбиение (quickselect)? Что с ожидаемым и с худшим временем, и как выбирать опорный элемент?
5. Что случится на массиве из одинаковых значений? Как поведёт себя ваше разбиение?
6. Что произойдёт, если массив уже отсортирован? Что вы предложите?

По плану это суббота, первая из трёх задач.

**Шаблон с тестами.**

```cpp
namespace {
	int find_kth_largest([[maybe_unused]] std::vector<int>& nums,
						 [[maybe_unused]] int k) {
		return 0;
	}

	void test_kth(const std::string& label,
				  std::vector<int> nums,
				  int k) {
		std::vector<int> sorted{nums};
		std::ranges::sort(sorted);
		const int expected{sorted[sorted.size() - static_cast<std::size_t>(k)]};

		const int result{find_kth_largest(nums, k)};

		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_kth("description example 1",
		{3, 2, 1, 5, 6, 4},
		2);
	test_kth("description example 2",
		{3, 2, 3, 1, 2, 4, 5, 5, 6},
		4);
	test_kth("single element",
		{7},
		1);
	test_kth("single negative element",
		{-7},
		1);
	test_kth("two elements, k = 1",
		{1, 2},
		1);
	test_kth("two elements, k = 2",
		{2, 1},
		2);
	test_kth("two equal elements",
		{5, 5},
		2);
	test_kth("k = 1, the maximum",
		{4, 9, 1, 7, 3},
		1);
	test_kth("k = n, the minimum",
		{4, 9, 1, 7, 3},
		5);
	test_kth("all equal values",
		{6, 6, 6, 6, 6, 6},
		3);
	test_kth("duplicates of the maximum, k = 2",
		{9, 9, 9, 1, 2},
		2);
	test_kth("duplicates of the maximum, k = 3",
		{9, 9, 9, 1, 2},
		3);
	test_kth("duplicates of the maximum, k = 4",
		{9, 9, 9, 1, 2},
		4);
	test_kth("duplicates of the minimum, k = n",
		{3, 1, 1, 1, 2},
		5);
	test_kth("duplicates of the minimum, k = n - 1",
		{3, 1, 1, 1, 2},
		4);
	test_kth("already sorted ascending",
		{1, 2, 3, 4, 5, 6, 7, 8},
		3);
	test_kth("already sorted descending",
		{8, 7, 6, 5, 4, 3, 2, 1},
		3);
	test_kth("sorted ascending, k = n",
		{1, 2, 3, 4, 5, 6, 7, 8},
		8);
	test_kth("sorted descending, k = 1",
		{8, 7, 6, 5, 4, 3, 2, 1},
		1);
	test_kth("negative values",
		{-5, -1, -9, -3, -7},
		2);
	test_kth("mixed signs with zero",
		{0, -1, 1, 0, -2, 2},
		3);
	test_kth("extreme values, k = 1",
		{-1000000000, 1000000000, 0},
		1);
	test_kth("extreme values, k = 3",
		{-1000000000, 1000000000, 0},
		3);
	test_kth("extreme values duplicated",
		{1000000000, 1000000000, -1000000000, -1000000000},
		3);
	test_kth("two distinct values",
		{1, 2, 1, 2, 1, 2, 1, 2},
		5);
	test_kth("three distinct values",
		{2, 0, 1, 2, 0, 1, 2, 0, 1},
		4);
	test_kth("median of an odd array",
		{5, 3, 8, 1, 9, 2, 7},
		4);
	test_kth("median of an even array, k = n / 2",
		{5, 3, 8, 1, 9, 2, 7, 4},
		4);
	test_kth("median of an even array, k = n / 2 + 1",
		{5, 3, 8, 1, 9, 2, 7, 4},
		5);
	test_kth("pivot in the middle is the answer",
		{1, 3, 2, 4, 5},
		3);
	test_kth("answer is repeated and not the extreme",
		{1, 5, 5, 5, 9},
		3);
	test_kth("random 1, n=4, k=1",
		{2, -5, 1, -1},
		1);
	test_kth("random 2, n=4, k=2",
		{4, 1, -8, -5},
		2);
	test_kth("random 3, n=11, k=11",
		{-5, 8, 6, 5, 8, -4, 6, 5, -1, 5, -4},
		11);
	test_kth("random 4, n=11, k=4",
		{7, 2, -1, -7, 8, -2, -4, -1, 2, -7, -7},
		4);
	test_kth("random 5, n=16, k=16",
		{-1, 0, -8, 5, -3, 3, -8, -2, 3, 8, 8, 3, 8, -4, -8, -1},
		16);
	test_kth("random 6, n=18, k=10",
		{8, 3, 2, -3, 2, -3, 1, -5, 1, 3, 2, -7, 2, -7, 8, -6, -1, -6},
		10);
	test_kth("random 7, n=20, k=17",
		{8, -7, 4, -4, 8, -6, 5, 2, -7, -2, -4, -3, -2, -5, -3, -2, 7, 6, -3, -2},
		17);
	test_kth("random 8, n=17, k=12",
		{0, 8, -1, 0, -8, -4, 3, -5, 5, -2, -2, -8, 1, 5, -6, 3, 3},
		12);

	{
		// n = 1000000, pseudo-random values, k = 1, 500000, 1000000
		std::vector<int> nums(1000000);
		unsigned state{2024};
		for (auto& x : nums) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 4) % 2000001u) - 1000000;
		}
		test_kth("n = 1000000, random, k = 1", nums, 1);
		test_kth("n = 1000000, random, k = 500000", nums, 500000);
		test_kth("n = 1000000, random, k = 1000000", nums, 1000000);
	}
	{
		// n = 1000000, sorted ascending, k = n / 2
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i;
		test_kth("n = 1000000, sorted ascending", nums, 500000);
	}
	{
		// n = 1000000, sorted descending, k = n / 2
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = 1000000 - i;
		test_kth("n = 1000000, sorted descending", nums, 500000);
	}
	{
		// n = 1000000, all equal
		test_kth("n = 1000000, all equal", std::vector<int>(1000000, 17), 500000);
	}
	{
		// n = 1000000, only two distinct values
		std::vector<int> nums(1000000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = (i % 2 == 0) ? 1 : 2;
		test_kth("n = 1000000, two distinct values", nums, 600000);
	}
	{
		// n = 1000000, values 0..9 only (many duplicates)
		std::vector<int> nums(1000000);
		unsigned state{99};
		for (auto& x : nums) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 16) % 10u);
		}
		test_kth("n = 1000000, ten distinct values", nums, 333333);
	}
	{
		// n = 1000000, organ pipe: ascending then descending
		std::vector<int> nums(1000000);
		for (int i{0}; i < 500000; ++i) {
			nums[static_cast<std::size_t>(i)] = i;
			nums[static_cast<std::size_t>(999999 - i)] = i;
		}
		test_kth("n = 1000000, organ pipe", nums, 250000);
	}
	{
		// n = 1000000, sawtooth with period 1000
		std::vector<int> nums(1000000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = static_cast<int>(i % 1000);
		test_kth("n = 1000000, sawtooth", nums, 400000);
	}
	{
		// n = 1000000, one very large value among small ones, k = 1 and k = 2
		std::vector<int> nums(1000000, 5);
		nums[777777] = 1000000000;
		test_kth("n = 1000000, one huge value, k = 1", nums, 1);
		test_kth("n = 1000000, one huge value, k = 2", nums, 2);
	}

	return 0;
}
```

Тесты проверены запуском. Заглушка даёт FAIL на 50 из 51. Эталонное решение проходит 51 из 51 за 0,46 с, санитайзеры чистые. Решения через кучу и через полную сортировку тоже проходят, потому что тесты не отличают O(n) от O(n log n) по времени, и это отдельная тема для разговора с интервьюером. Ошибочные варианты ловятся: «k-й с конца» — 37 тестами, учёт только различных значений — 14, а квадратичные разбиения не укладываются в 40 секунд на больших блоках.

Пришлите своё решение, и я разберу его. Перед отправкой соберите с `-Wall -Wextra -Wconversion -Wsign-conversion` и запустите тесты. Индексы, идущие вниз до нуля, лучше делать знаковыми.


---

work : 14:55  - 15:41

---

k = 2 ||| nums = 3, 2, 1, 5, 6, 4 ||| sort = 1, 2, 3, 4, 5, 6  => kth == 2
[] - i
() - j

- left = 0, r = 5, pivot = 4
- [(3)], 2, 1, 5, 6, 4
- ...
- 3, 2, 1, [(5)], 6, 4
- 3, 2, 1, [5], (6), 4
- 3, 2, 1, [5], 6, (4)
- swap i & right => 3, 2, 1, [4], 6, (5)
- i = 3 ( < 2) => right - 1 = 4, left = 0, pivot = 6
- {[(3)], 2, 1, 4, 6}, 5
- ...
- {3, 2, 1, 4, [(6)]}, 5
- i = 4 (< 2) => right - 1 = 3, left = 0, pivot = 4
- {[(3)], 2, 1, 4}, 6, 5
- ...
- {3, 2, 1, [(4)]}, 6, 5
- i = 3 (< 2) => right - 1 = 2, left = 0, pivot = 1
- {[(3)], 2, 1}, 4, 6, 5
- ...
- {[3], 2, (1)}, 4, 6, 5
- swap i & right = > {[1], 2, (3)}, 4, 6, 5
- i = 0 ( > 2)  => right = 2, left + 1 = 1, pivot = 3
- 1, {[(2)], 3}, 4, 6, 5
- 1, {2, [(3)]}, 4, 6, 5
- i = 2 ( == k) => result = 3



---


```cpp
#include <iostream>
#include <format>
#include <vector>
#include <string>
#include <cstddef>
#include <algorithm>
#include <random>
#include <queue>

namespace {
	constexpr int BAD_RESULT{-1};

	int find_kth_largest(std::vector<int>& nums,
						 int k) {

        std::mt19937 gen{42};
        std::size_t left{0};
        std::size_t right{nums.size()};
        const std::size_t target{static_cast<std::size_t>(k-1)};
        while (true) {
            const std::size_t pivot_idx{left + gen() % (right - left)};
            const int pivot{nums[pivot_idx]};

            std::size_t lo{left};
            std::size_t i{left};
            std::size_t hi{right};

            while (i < hi) {
                if (nums[i] > pivot) {
                    std::swap(nums[lo++], nums[i++]);
                } else if (nums[i] == pivot) {
                    ++i;
                } else {
                    std::swap(nums[i], nums[--hi]);
                }
            }
            if (target < lo) {
	            right = lo;
	        }
            else if (target < hi) {
	            return pivot;
            }
            else {
	            left = hi;
            }
        }

        return BAD_RESULT;
	}
	
	int find_kth_largest_pq(std::vector<int>& nums,
						    int k) {
		
		std::priority_queue<int, std::vector<int>, std::greater<>> pq;
		for (const auto num: nums) {
			pq.push(num);
			if (pq.size() > static_cast<std::size_t>(k)) {
				pq.pop();
			}
		}
		
		return pq.size() != 0 ? pq.top() : BAD_RESULT;
	}

	void test_kth(const std::string& label,
				  std::vector<int> nums,
				  int k) {
		std::vector<int> sorted{nums};
		std::ranges::sort(sorted);
		const int expected{sorted[sorted.size() - static_cast<std::size_t>(k)]};

		const int result{find_kth_largest(nums, k)};
		const int result_pq{find_kth_largest_pq(nums, k)};

		std::cout << std::format("{}: 1: {}, 2: {}\n",
			label,
			result == expected ? "OK" : "FAIL",
			result_pq == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_kth("description example 1",
		{3, 2, 1, 5, 6, 4},
		2);
	test_kth("description example 2",
		{3, 2, 3, 1, 2, 4, 5, 5, 6},
		4);
	test_kth("single element",
		{7},
		1);
	test_kth("single negative element",
		{-7},
		1);
	test_kth("two elements, k = 1",
		{1, 2},
		1);
	test_kth("two elements, k = 2",
		{2, 1},
		2);
	test_kth("two equal elements",
		{5, 5},
		2);
	test_kth("k = 1, the maximum",
		{4, 9, 1, 7, 3},
		1);
	test_kth("k = n, the minimum",
		{4, 9, 1, 7, 3},
		5);
	test_kth("all equal values",
		{6, 6, 6, 6, 6, 6},
		3);
	test_kth("duplicates of the maximum, k = 2",
		{9, 9, 9, 1, 2},
		2);
	test_kth("duplicates of the maximum, k = 3",
		{9, 9, 9, 1, 2},
		3);
	test_kth("duplicates of the maximum, k = 4",
		{9, 9, 9, 1, 2},
		4);
	test_kth("duplicates of the minimum, k = n",
		{3, 1, 1, 1, 2},
		5);
	test_kth("duplicates of the minimum, k = n - 1",
		{3, 1, 1, 1, 2},
		4);
	test_kth("already sorted ascending",
		{1, 2, 3, 4, 5, 6, 7, 8},
		3);
	test_kth("already sorted descending",
		{8, 7, 6, 5, 4, 3, 2, 1},
		3);
	test_kth("sorted ascending, k = n",
		{1, 2, 3, 4, 5, 6, 7, 8},
		8);
	test_kth("sorted descending, k = 1",
		{8, 7, 6, 5, 4, 3, 2, 1},
		1);
	test_kth("negative values",
		{-5, -1, -9, -3, -7},
		2);
	test_kth("mixed signs with zero",
		{0, -1, 1, 0, -2, 2},
		3);
	test_kth("extreme values, k = 1",
		{-1000000000, 1000000000, 0},
		1);
	test_kth("extreme values, k = 3",
		{-1000000000, 1000000000, 0},
		3);
	test_kth("extreme values duplicated",
		{1000000000, 1000000000, -1000000000, -1000000000},
		3);
	test_kth("two distinct values",
		{1, 2, 1, 2, 1, 2, 1, 2},
		5);
	test_kth("three distinct values",
		{2, 0, 1, 2, 0, 1, 2, 0, 1},
		4);
	test_kth("median of an odd array",
		{5, 3, 8, 1, 9, 2, 7},
		4);
	test_kth("median of an even array, k = n / 2",
		{5, 3, 8, 1, 9, 2, 7, 4},
		4);
	test_kth("median of an even array, k = n / 2 + 1",
		{5, 3, 8, 1, 9, 2, 7, 4},
		5);
	test_kth("pivot in the middle is the answer",
		{1, 3, 2, 4, 5},
		3);
	test_kth("answer is repeated and not the extreme",
		{1, 5, 5, 5, 9},
		3);
	test_kth("random 1, n=4, k=1",
		{2, -5, 1, -1},
		1);
	test_kth("random 2, n=4, k=2",
		{4, 1, -8, -5},
		2);
	test_kth("random 3, n=11, k=11",
		{-5, 8, 6, 5, 8, -4, 6, 5, -1, 5, -4},
		11);
	test_kth("random 4, n=11, k=4",
		{7, 2, -1, -7, 8, -2, -4, -1, 2, -7, -7},
		4);
	test_kth("random 5, n=16, k=16",
		{-1, 0, -8, 5, -3, 3, -8, -2, 3, 8, 8, 3, 8, -4, -8, -1},
		16);
	test_kth("random 6, n=18, k=10",
		{8, 3, 2, -3, 2, -3, 1, -5, 1, 3, 2, -7, 2, -7, 8, -6, -1, -6},
		10);
	test_kth("random 7, n=20, k=17",
		{8, -7, 4, -4, 8, -6, 5, 2, -7, -2, -4, -3, -2, -5, -3, -2, 7, 6, -3, -2},
		17);
	test_kth("random 8, n=17, k=12",
		{0, 8, -1, 0, -8, -4, 3, -5, 5, -2, -2, -8, 1, 5, -6, 3, 3},
		12);

	{
		// n = 1000000, pseudo-random values, k = 1, 500000, 1000000
		std::vector<int> nums(1000000);
		unsigned state{2024};
		for (auto& x : nums) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 4) % 2000001u) - 1000000;
		}
		test_kth("n = 1000000, random, k = 1", nums, 1);
		test_kth("n = 1000000, random, k = 500000", nums, 500000);
		test_kth("n = 1000000, random, k = 1000000", nums, 1000000);
	}
	{
		// n = 1000000, sorted ascending, k = n / 2
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i;
		test_kth("n = 1000000, sorted ascending", nums, 500000);
	}
	{
		// n = 1000000, sorted descending, k = n / 2
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = 1000000 - i;
		test_kth("n = 1000000, sorted descending", nums, 500000);
	}
	{
		// n = 1000000, all equal
		test_kth("n = 1000000, all equal", std::vector<int>(1000000, 17), 500000);
	}
	{
		// n = 1000000, only two distinct values
		std::vector<int> nums(1000000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = (i % 2 == 0) ? 1 : 2;
		test_kth("n = 1000000, two distinct values", nums, 600000);
	}
	{
		// n = 1000000, values 0..9 only (many duplicates)
		std::vector<int> nums(1000000);
		unsigned state{99};
		for (auto& x : nums) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 16) % 10u);
		}
		test_kth("n = 1000000, ten distinct values", nums, 333333);
	}
	{
		// n = 1000000, organ pipe: ascending then descending
		std::vector<int> nums(1000000);
		for (int i{0}; i < 500000; ++i) {
			nums[static_cast<std::size_t>(i)] = i;
			nums[static_cast<std::size_t>(999999 - i)] = i;
		}
		test_kth("n = 1000000, organ pipe", nums, 250000);
	}
	{
		// n = 1000000, sawtooth with period 1000
		std::vector<int> nums(1000000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = static_cast<int>(i % 1000);
		test_kth("n = 1000000, sawtooth", nums, 400000);
	}
	{
		// n = 1000000, one very large value among small ones, k = 1 and k = 2
		std::vector<int> nums(1000000, 5);
		nums[777777] = 1000000000;
		test_kth("n = 1000000, one huge value, k = 1", nums, 1);
		test_kth("n = 1000000, one huge value, k = 2", nums, 2);
	}

	return 0;
}
```