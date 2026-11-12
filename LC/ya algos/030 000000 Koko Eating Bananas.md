**Задача 3 блока 5 (бинарный поиск): Koko Eating Bananas**

Это новый приём: бинарный поиск не по индексу массива, а по значению ответа.

**Условие.** Коко любит бананы. Перед ней `n` куч: в `piles[i]` лежит `piles[i]` бананов. Охранники вернутся через `h` часов. Коко выбирает скорость `k` (бананов в час) и каждый час ест из одной кучи: если в куче меньше `k` бананов, она доедает кучу и до конца часа больше ничего не ест. Найдите минимальное целое `k`, при котором она успеет съесть все бананы за `h` часов.

**Сигнатура:**

```cpp
int min_eating_speed(const std::vector<int>& piles, int h);
```

**Ограничения:**

- 1 ≤ n ≤ 100 000
- 1 ≤ piles[i] ≤ 10^9
- n ≤ h ≤ 2 147 483 647

**Примеры:**

- `piles = [3, 6, 7, 11], h = 8` → `4`
- `piles = [30, 11, 23, 4, 20], h = 5` → `30`
- `piles = [30, 11, 23, 4, 20], h = 6` → `23`

**Вопросы интервьюера (ответьте после решения):**

1. Почему здесь работает бинарный поиск, хотя массив `piles` не отсортирован? По какой функции вы ищете и чем она монотонна?
2. Какие границы `lo` и `hi` вы выбрали и почему именно они? Что будет с `lo = 0`?
3. Как вы считаете часы на одну кучу? Какой тип вы взяли для суммы часов и при каких входных данных он переполнится?
4. Какая итоговая сложность по времени? Сравните с перебором всех скоростей подряд.

**Цель:** O(n · log(max(piles))) по времени, O(1) по памяти.

**Шаблон:**

```cpp
namespace {
	int min_eating_speed([[maybe_unused]] const std::vector<int>& piles,
						 [[maybe_unused]] int h) {
		return 0;
	}

	long long hours_needed(const std::vector<int>& piles, long long speed) {
		long long hours{0};
		for (const int pile : piles) hours += (pile + speed - 1) / speed;
		return hours;
	}

	int expected_speed(const std::vector<int>& piles, int h) {
		// the smallest speed k in [1, max pile] with hours_needed(k) <= h
		const int largest{*std::ranges::max_element(piles)};
		const auto speeds{std::views::iota(1, largest + 1)};
		return *std::ranges::partition_point(speeds, [&](int speed) {
			return hours_needed(piles, speed) > h;
		});
	}

	void test_eating(const std::string& label,
					 const std::vector<int>& piles,
					 int h) {
		const auto expected{expected_speed(piles, h)};

		const auto result{min_eating_speed(piles, h)};

		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_eating("description example 1",
		{3, 6, 7, 11},
		8);
	test_eating("description example 2",
		{30, 11, 23, 4, 20},
		5);
	test_eating("description example 3",
		{30, 11, 23, 4, 20},
		6);
	test_eating("single pile, one hour",
		{10},
		1);
	test_eating("single pile, many hours",
		{10},
		10);
	test_eating("single pile, more hours than bananas",
		{10},
		25);
	test_eating("single pile of one banana",
		{1},
		1);
	test_eating("single pile of one banana, many hours",
		{1},
		100);
	test_eating("hours equal to the number of piles",
		{5, 5, 5, 5},
		4);
	test_eating("hours equal to the number of piles, uneven",
		{1, 9, 3, 7},
		4);
	test_eating("one hour per pile with the largest pile first",
		{100, 1, 1},
		3);
	test_eating("one hour per pile with the largest pile last",
		{1, 1, 100},
		3);
	test_eating("all piles of one banana",
		{1, 1, 1, 1, 1},
		5);
	test_eating("all piles of one banana, extra hours",
		{1, 1, 1, 1, 1},
		50);
	test_eating("all piles equal, tight",
		{6, 6, 6},
		6);
	test_eating("all piles equal, exact fit",
		{6, 6, 6},
		9);
	test_eating("all piles equal, loose",
		{6, 6, 6},
		18);
	test_eating("ceil matters: 7 and 7 in 4 hours",
		{7, 7},
		4);
	test_eating("floor division would give a smaller speed",
		{7, 7},
		3);
	test_eating("answer is 1",
		{2, 2, 2},
		6);
	test_eating("answer is 2",
		{2, 2, 2},
		3);
	test_eating("answer is the maximum pile",
		{3, 9, 4},
		3);
	test_eating("answer is one less than the maximum pile",
		{3, 9, 4},
		4);
	test_eating("two piles, wide gap",
		{1, 1000},
		3);
	test_eating("two piles, wide gap, many hours",
		{1, 1000},
		500);
	test_eating("increasing piles",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
		15);
	test_eating("decreasing piles",
		{10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
		15);
	test_eating("increasing piles, many hours",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
		55);
	test_eating("piles of two sizes",
		{2, 2, 9, 9, 2, 9},
		8);
	test_eating("large piles, few hours",
		{1000000000, 1000000000},
		2);
	test_eating("large piles, more hours",
		{1000000000, 1000000000},
		4);
	test_eating("large piles, many hours",
		{1000000000},
		1000000000);
	test_eating("large piles, one hour more than piles",
		{1000000000, 1000000000, 1000000000},
		4);
	test_eating("large pile and tiny piles",
		{1000000000, 1, 1, 1},
		5);
	test_eating("large pile and tiny piles, tight",
		{1000000000, 1, 1, 1},
		4);
	test_eating("prime sized piles",
		{2, 3, 5, 7, 11, 13},
		10);
	test_eating("powers of two",
		{1, 2, 4, 8, 16, 32, 64},
		14);
	test_eating("hours barely enough for speed 2",
		{3, 3, 3, 3},
		8);
	test_eating("hours not enough for speed 2",
		{3, 3, 3, 3},
		7);
	test_eating("random 1, n=9",
		{9, 24, 29, 26, 11, 14, 26, 20, 8},
		69);
	test_eating("random 2, n=2",
		{12, 23},
		9);
	test_eating("random 3, n=9",
		{15, 8, 4, 24, 29, 27, 2, 20, 27},
		31);
	test_eating("random 4, n=1",
		{21},
		2);
	test_eating("random 5, n=8",
		{22, 29, 18, 9, 4, 3, 1, 30},
		17);
	test_eating("random 6, n=10",
		{23, 23, 21, 15, 27, 21, 11, 9, 5, 24},
		47);
	test_eating("random 7, n=5",
		{27, 25, 14, 14, 13},
		25);
	test_eating("random 8, n=7",
		{5, 27, 16, 4, 4, 19, 19},
		53);

	{
		// n = 100000, pseudo-random piles in [1, 1000000000], h = n (the answer is the largest pile)
		std::vector<int> piles(100000);
		unsigned state{875};
		for (auto& pile : piles) {
			state = state * 1103515245u + 12345u;
			pile = static_cast<int>((state >> 2) % 1000000000u) + 1;
		}
		test_eating("n = 100000, random piles, h = n", piles, 100000);
		test_eating("n = 100000, random piles, h = 2n", piles, 200000);
		test_eating("n = 100000, random piles, h = 1000000", piles, 1000000);
		test_eating("n = 100000, random piles, h = 1000000000", piles, 1000000000);
		test_eating("n = 100000, random piles, h = 2147483647", piles, 2147483647);
	}
	{
		// n = 100000, all piles are 1000000000: the total hours overflow int for small speeds
		const std::vector<int> piles(100000, 1000000000);
		test_eating("n = 100000, all piles 1000000000, h = 1000000000", piles, 1000000000);
		test_eating("n = 100000, all piles 1000000000, h = 100000", piles, 100000);
		test_eating("n = 100000, all piles 1000000000, h = 100001", piles, 100001);
		test_eating("n = 100000, all piles 1000000000, h = 2147483647", piles, 2147483647);
	}
	{
		// n = 100000, all piles are 1
		const std::vector<int> piles(100000, 1);
		test_eating("n = 100000, all piles 1, h = n", piles, 100000);
		test_eating("n = 100000, all piles 1, h = 1000000000", piles, 1000000000);
	}
	{
		// n = 100000, pseudo-random small piles in [1, 50]
		std::vector<int> piles(100000);
		unsigned state{11};
		for (auto& pile : piles) {
			state = state * 1103515245u + 12345u;
			pile = static_cast<int>((state >> 8) % 50u) + 1;
		}
		test_eating("n = 100000, small piles, h = n", piles, 100000);
		test_eating("n = 100000, small piles, h = 3n", piles, 300000);
		test_eating("n = 100000, small piles, h = 1000000000", piles, 1000000000);
		test_eating("n = 100000, small piles, h = 2147483647", piles, 2147483647);
	}
	{
		// n = 1, the extreme single piles
		test_eating("single pile 1000000000, h = 1", {1000000000}, 1);
		test_eating("single pile 1000000000, h = 1000000000", {1000000000}, 1000000000);
		test_eating("single pile 1000000000, h = 3", {1000000000}, 3);
		test_eating("single pile 1000000000, h = 2147483647", {1000000000}, 2147483647);
	}
	{
		// n = 100000, one huge pile among ones
		std::vector<int> piles(100000, 1);
		piles[54321] = 1000000000;
		test_eating("n = 100000, one huge pile, h = n", piles, 100000);
		test_eating("n = 100000, one huge pile, h = n + 1", piles, 100001);
		test_eating("n = 100000, one huge pile, h = n + 1000", piles, 101000);
	}

	return 0;
}
```

Тесты проверены запуском:

- Заглушка даёт FAIL на всех 69.
- Эталон проходит 69 из 69, санитайзеры чистые.
- Ожидаемые значения 41 маленького теста я сверил с перебором на Python, расхождений нет.
- Неверные варианты ловятся: целочисленное деление вместо потолочного, `hi = max - 1`, строгое `<` вместо `<=` при проверке часов, формула «сумма деленная на часы», линейный перебор скоростей (таймаут на больших тестах), `lo = 0` (деление на ноль).
- Сумма часов в `int` ловится двумя тестами с `h = 2147483647` (в них переполнение происходит в самом поиске).

---

work 21:48 - 22:29

---

- `piles = [3, 6, 7, 11], h = 8` → `4`
- {3, 6, 7, 11} & h = 8 \\ min_speed = 1 , max_speed = 11
-    => mid  = 1 + (11 - 1) / 2 = 6 =>  {1, 1, 2, 2} = 6
-    => min_speed = 1, max_speed = 6 => 1 + (6 - 1) / 2 => 3
-    => min_speed = 2, max_speed = 6, mid_speed = 2 + (6 - 2) / 2 => 3
-    => mid_speed = 3 => {1,  2, 3, 4} => 10
-    => min_speed = 3 = > 3 + (6 - 3) / 2 = >4
-    => mid_speed = 4 => {1, 2, 2, 3} +++

---


```cpp
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <cstddef>
#include <cmath>
#include <algorithm>

namespace {
	int min_eating_speed(const std::vector<int>& piles,
						 int h) {
		const auto max_it{std::max_element(piles.begin(), piles.end())};
		int min_speed{1};
		int max_speed{*max_it};
		
		const auto calculate_total_hours = [&piles] (int mid_speed) -> long long {
			long long total_hours{0};
			for (const auto pile: piles) {
				total_hours += static_cast<long long>((pile + mid_speed - 1) / mid_speed);
			}
			
			return total_hours;
		};
		
		while (min_speed < max_speed) {
			const auto mid_speed = min_speed + (max_speed - min_speed) / 2;
			const auto total_hours{calculate_total_hours(mid_speed)};
			
			if (total_hours <= static_cast<long long>(h)) {
				max_speed = mid_speed;
			} else {
				min_speed = mid_speed + 1;
			}
		}

		return mid_speed;
	}

	long long hours_needed(const std::vector<int>& piles, long long speed) {
		long long hours{0};
		for (const int pile : piles) hours += (pile + speed - 1) / speed;
		return hours;
	}

	int expected_speed(const std::vector<int>& piles, int h) {
		// the smallest speed k in [1, max pile] with hours_needed(k) <= h
		const int largest{*std::ranges::max_element(piles)};
		const auto speeds{std::views::iota(1, largest + 1)};
		return *std::ranges::partition_point(speeds, [&](int speed) {
			return hours_needed(piles, speed) > h;
		});
	}

	void test_eating(const std::string& label,
					 const std::vector<int>& piles,
					 int h) {
		const auto expected{expected_speed(piles, h)};

		const auto result{min_eating_speed(piles, h)};

		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_eating("description example 1",
		{3, 6, 7, 11},
		8);
	test_eating("description example 2",
		{30, 11, 23, 4, 20},
		5);
	test_eating("description example 3",
		{30, 11, 23, 4, 20},
		6);
	test_eating("single pile, one hour",
		{10},
		1);
	test_eating("single pile, many hours",
		{10},
		10);
	test_eating("single pile, more hours than bananas",
		{10},
		25);
	test_eating("single pile of one banana",
		{1},
		1);
	test_eating("single pile of one banana, many hours",
		{1},
		100);
	test_eating("hours equal to the number of piles",
		{5, 5, 5, 5},
		4);
	test_eating("hours equal to the number of piles, uneven",
		{1, 9, 3, 7},
		4);
	test_eating("one hour per pile with the largest pile first",
		{100, 1, 1},
		3);
	test_eating("one hour per pile with the largest pile last",
		{1, 1, 100},
		3);
	test_eating("all piles of one banana",
		{1, 1, 1, 1, 1},
		5);
	test_eating("all piles of one banana, extra hours",
		{1, 1, 1, 1, 1},
		50);
	test_eating("all piles equal, tight",
		{6, 6, 6},
		6);
	test_eating("all piles equal, exact fit",
		{6, 6, 6},
		9);
	test_eating("all piles equal, loose",
		{6, 6, 6},
		18);
	test_eating("ceil matters: 7 and 7 in 4 hours",
		{7, 7},
		4);
	test_eating("floor division would give a smaller speed",
		{7, 7},
		3);
	test_eating("answer is 1",
		{2, 2, 2},
		6);
	test_eating("answer is 2",
		{2, 2, 2},
		3);
	test_eating("answer is the maximum pile",
		{3, 9, 4},
		3);
	test_eating("answer is one less than the maximum pile",
		{3, 9, 4},
		4);
	test_eating("two piles, wide gap",
		{1, 1000},
		3);
	test_eating("two piles, wide gap, many hours",
		{1, 1000},
		500);
	test_eating("increasing piles",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
		15);
	test_eating("decreasing piles",
		{10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
		15);
	test_eating("increasing piles, many hours",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
		55);
	test_eating("piles of two sizes",
		{2, 2, 9, 9, 2, 9},
		8);
	test_eating("large piles, few hours",
		{1000000000, 1000000000},
		2);
	test_eating("large piles, more hours",
		{1000000000, 1000000000},
		4);
	test_eating("large piles, many hours",
		{1000000000},
		1000000000);
	test_eating("large piles, one hour more than piles",
		{1000000000, 1000000000, 1000000000},
		4);
	test_eating("large pile and tiny piles",
		{1000000000, 1, 1, 1},
		5);
	test_eating("large pile and tiny piles, tight",
		{1000000000, 1, 1, 1},
		4);
	test_eating("prime sized piles",
		{2, 3, 5, 7, 11, 13},
		10);
	test_eating("powers of two",
		{1, 2, 4, 8, 16, 32, 64},
		14);
	test_eating("hours barely enough for speed 2",
		{3, 3, 3, 3},
		8);
	test_eating("hours not enough for speed 2",
		{3, 3, 3, 3},
		7);
	test_eating("random 1, n=9",
		{9, 24, 29, 26, 11, 14, 26, 20, 8},
		69);
	test_eating("random 2, n=2",
		{12, 23},
		9);
	test_eating("random 3, n=9",
		{15, 8, 4, 24, 29, 27, 2, 20, 27},
		31);
	test_eating("random 4, n=1",
		{21},
		2);
	test_eating("random 5, n=8",
		{22, 29, 18, 9, 4, 3, 1, 30},
		17);
	test_eating("random 6, n=10",
		{23, 23, 21, 15, 27, 21, 11, 9, 5, 24},
		47);
	test_eating("random 7, n=5",
		{27, 25, 14, 14, 13},
		25);
	test_eating("random 8, n=7",
		{5, 27, 16, 4, 4, 19, 19},
		53);

	{
		// n = 100000, pseudo-random piles in [1, 1000000000], h = n (the answer is the largest pile)
		std::vector<int> piles(100000);
		unsigned state{875};
		for (auto& pile : piles) {
			state = state * 1103515245u + 12345u;
			pile = static_cast<int>((state >> 2) % 1000000000u) + 1;
		}
		test_eating("n = 100000, random piles, h = n", piles, 100000);
		test_eating("n = 100000, random piles, h = 2n", piles, 200000);
		test_eating("n = 100000, random piles, h = 1000000", piles, 1000000);
		test_eating("n = 100000, random piles, h = 1000000000", piles, 1000000000);
		test_eating("n = 100000, random piles, h = 2147483647", piles, 2147483647);
	}
	{
		// n = 100000, all piles are 1000000000: the total hours overflow int for small speeds
		const std::vector<int> piles(100000, 1000000000);
		test_eating("n = 100000, all piles 1000000000, h = 1000000000", piles, 1000000000);
		test_eating("n = 100000, all piles 1000000000, h = 100000", piles, 100000);
		test_eating("n = 100000, all piles 1000000000, h = 100001", piles, 100001);
		test_eating("n = 100000, all piles 1000000000, h = 2147483647", piles, 2147483647);
	}
	{
		// n = 100000, all piles are 1
		const std::vector<int> piles(100000, 1);
		test_eating("n = 100000, all piles 1, h = n", piles, 100000);
		test_eating("n = 100000, all piles 1, h = 1000000000", piles, 1000000000);
	}
	{
		// n = 100000, pseudo-random small piles in [1, 50]
		std::vector<int> piles(100000);
		unsigned state{11};
		for (auto& pile : piles) {
			state = state * 1103515245u + 12345u;
			pile = static_cast<int>((state >> 8) % 50u) + 1;
		}
		test_eating("n = 100000, small piles, h = n", piles, 100000);
		test_eating("n = 100000, small piles, h = 3n", piles, 300000);
		test_eating("n = 100000, small piles, h = 1000000000", piles, 1000000000);
		test_eating("n = 100000, small piles, h = 2147483647", piles, 2147483647);
	}
	{
		// n = 1, the extreme single piles
		test_eating("single pile 1000000000, h = 1", {1000000000}, 1);
		test_eating("single pile 1000000000, h = 1000000000", {1000000000}, 1000000000);
		test_eating("single pile 1000000000, h = 3", {1000000000}, 3);
		test_eating("single pile 1000000000, h = 2147483647", {1000000000}, 2147483647);
	}
	{
		// n = 100000, one huge pile among ones
		std::vector<int> piles(100000, 1);
		piles[54321] = 1000000000;
		test_eating("n = 100000, one huge pile, h = n", piles, 100000);
		test_eating("n = 100000, one huge pile, h = n + 1", piles, 100001);
		test_eating("n = 100000, one huge pile, h = n + 1000", piles, 101000);
	}

	return 0;
}
```