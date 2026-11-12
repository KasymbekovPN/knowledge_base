## Блок 4 (сортировки), задача 2: 88. Merge Sorted Array

**Условие.** Даны два массива, отсортированных по неубыванию: `nums1` и `nums2`.

- В `nums1` лежит `m + n` элементов. Первые `m` из них — настоящие значения, остальные `n` — заглушки (нули).
- В `nums2` лежит `n` значений.

Нужно слить `nums2` в `nums1` так, чтобы `nums1` стал отсортированным по неубыванию целиком. Функция ничего не возвращает, результат остаётся в `nums1`.

Библиотечные функции для слияния и сортировки (`std::sort`, `std::merge`, `std::inplace_merge` и так далее) использовать нельзя.

**Сигнатура.**

```cpp
void merge_sorted(std::vector<int>& nums1, int m, const std::vector<int>& nums2, int n);
```

**Ограничения.**

- 0 ≤ m, n ≤ 10⁶
- `nums1.size() == m + n`, `nums2.size() == n`
- −10⁹ ≤ значения ≤ 10⁹
- оба массива отсортированы по неубыванию (в `nums1` — только первые `m` элементов)

**Целевая сложность.** По времени O(m + n), по дополнительной памяти O(1).

**Вопросы интервьюера** (вслух, в процессе):

1. Как решить проще всего? Что с временем и с памятью?
2. Если можно взять временный буфер, как слить за O(m + n)? Что именно мешает обойтись без него при слиянии «слева направо»?
3. Есть ли порядок обхода, при котором ничего не перезаписывается до того, как будет прочитано?
4. Когда один из массивов закончился, оставшиеся элементы какого из них нужно допереносить, а какого нет? Почему?
5. Как поведёт себя решение при `m == 0`, `n == 0` и при равных значениях в обоих массивах?

По плану это четверг, вторая задача блока 4.

**Шаблон с тестами.**

```cpp
namespace {
	void merge_sorted([[maybe_unused]] std::vector<int>& nums1,
					  [[maybe_unused]] int m,
					  [[maybe_unused]] const std::vector<int>& nums2,
					  [[maybe_unused]] int n) {
	}

	void test_merge(const std::string& label,
					const std::vector<int>& first,
					const std::vector<int>& second) {
		const int m{static_cast<int>(first.size())};
		const int n{static_cast<int>(second.size())};

		std::vector<int> expected{first};
		expected.insert(expected.end(), second.begin(), second.end());
		std::ranges::sort(expected);

		std::vector<int> nums1{first};
		nums1.resize(first.size() + second.size(), 0);

		merge_sorted(nums1, m, second, n);

		std::cout << std::format("{}: {}\n",
			label,
			nums1 == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_merge("description example 1",
		{1, 2, 3},
		{2, 5, 6});
	test_merge("description example 2",
		{1},
		{});
	test_merge("description example 3",
		{},
		{1});
	test_merge("both empty",
		{},
		{});
	test_merge("single elements, in order",
		{1},
		{2});
	test_merge("single elements, swapped",
		{2},
		{1});
	test_merge("single equal elements",
		{5},
		{5});
	test_merge("second array is empty",
		{1, 2, 3, 4},
		{});
	test_merge("first array is empty",
		{},
		{1, 2, 3, 4});
	test_merge("all of the second are smaller",
		{5, 6, 7},
		{1, 2, 3});
	test_merge("all of the second are greater",
		{1, 2, 3},
		{5, 6, 7});
	test_merge("interleaved one by one",
		{1, 3, 5, 7},
		{2, 4, 6, 8});
	test_merge("interleaved, second starts",
		{2, 4, 6, 8},
		{1, 3, 5, 7});
	test_merge("second fits in the middle",
		{1, 2, 9, 10},
		{4, 5});
	test_merge("second goes to both ends",
		{5, 6, 7},
		{1, 9});
	test_merge("second is longer",
		{4},
		{1, 2, 3, 5, 6});
	test_merge("first is longer",
		{1, 2, 3, 5, 6},
		{4});
	test_merge("all equal values",
		{7, 7, 7},
		{7, 7});
	test_merge("duplicates across arrays",
		{1, 2, 2, 3},
		{2, 2, 4});
	test_merge("negative values",
		{-5, -3, 0},
		{-4, -1, 2});
	test_merge("all negative, second smaller",
		{-3, -2, -1},
		{-9, -8});
	test_merge("zero values mixed with placeholders",
		{0, 0, 1},
		{0, 2});
	test_merge("zeros in both",
		{0, 0, 0},
		{0, 0});
	test_merge("extreme values",
		{-1000000000, 0, 1000000000},
		{-1000000000, 1000000000});
	test_merge("second is a single element, smallest",
		{2, 3, 4},
		{1});
	test_merge("second is a single element, largest",
		{2, 3, 4},
		{9});
	test_merge("first is a single element, smallest",
		{1},
		{2, 3, 4});
	test_merge("first is a single element, largest",
		{9},
		{2, 3, 4});
	test_merge("runs alternate in blocks",
		{1, 2, 3, 10, 11, 12},
		{4, 5, 6, 13, 14, 15});
	test_merge("remaining of second after first is exhausted",
		{3, 4},
		{1, 2, 5, 6, 7});
	test_merge("remaining of first after second is exhausted",
		{1, 2, 5, 6, 7},
		{3, 4});
	test_merge("random 1, m=6, n=3",
		{-6, -4, -1, 2, 4, 6},
		{-6, -1, 5});
	test_merge("random 2, m=10, n=7",
		{-5, -4, -4, -4, -3, -1, 0, 1, 3, 5},
		{-5, -4, -4, -2, 0, 1, 4});
	test_merge("random 3, m=13, n=13",
		{-6, -6, -4, -4, -3, -3, -3, -3, -1, 1, 1, 2, 4},
		{-6, -5, -4, 0, 0, 1, 1, 1, 2, 2, 2, 5, 6});
	test_merge("random 4, m=6, n=0",
		{-4, -4, -2, 0, 4, 5},
		{});
	test_merge("random 5, m=0, n=14",
		{},
		{-6, -5, -5, -4, -3, -2, -1, -1, 0, 0, 2, 2, 2, 5});
	test_merge("random 6, m=13, n=7",
		{-4, -2, -1, -1, -1, 0, 0, 3, 3, 3, 3, 4, 5},
		{-1, 0, 1, 2, 3, 6, 6});
	test_merge("random 7, m=0, n=10",
		{},
		{-6, -6, -3, -3, -2, -1, -1, 0, 0, 6});
	test_merge("random 8, m=8, n=7",
		{-6, -5, -2, 2, 5, 5, 6, 6},
		{-6, -4, -3, -2, -1, 2, 5});

	{
		// m = n = 1000000, interleaved: first holds evens, second holds odds
		std::vector<int> first(1000000);
		std::vector<int> second(1000000);
		for (int i{0}; i < 1000000; ++i) {
			first[static_cast<std::size_t>(i)] = 2 * i;
			second[static_cast<std::size_t>(i)] = 2 * i + 1;
		}
		test_merge("m = n = 1000000, interleaved", first, second);
	}
	{
		// m = n = 1000000, all of the second are smaller (worst case for shifting)
		std::vector<int> first(1000000);
		std::vector<int> second(1000000);
		for (int i{0}; i < 1000000; ++i) {
			first[static_cast<std::size_t>(i)] = 1000000 + i;
			second[static_cast<std::size_t>(i)] = i;
		}
		test_merge("m = n = 1000000, all of the second are smaller", first, second);
	}
	{
		// m = n = 1000000, all of the second are greater
		std::vector<int> first(1000000);
		std::vector<int> second(1000000);
		for (int i{0}; i < 1000000; ++i) {
			first[static_cast<std::size_t>(i)] = i;
			second[static_cast<std::size_t>(i)] = 1000000 + i;
		}
		test_merge("m = n = 1000000, all of the second are greater", first, second);
	}
	{
		// m = n = 1000000, pseudo-random sorted values, with duplicates
		std::vector<int> first(1000000);
		std::vector<int> second(1000000);
		unsigned state{777};
		for (auto& x : first) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 8) % 2000001u) - 1000000;
		}
		for (auto& x : second) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 8) % 2000001u) - 1000000;
		}
		std::ranges::sort(first);
		std::ranges::sort(second);
		test_merge("m = n = 1000000, random with duplicates", first, second);
	}
	{
		// m = 1000000, n = 0
		std::vector<int> first(1000000);
		for (int i{0}; i < 1000000; ++i) first[static_cast<std::size_t>(i)] = i;
		test_merge("m = 1000000, n = 0", first, {});
	}
	{
		// m = 0, n = 1000000
		std::vector<int> second(1000000);
		for (int i{0}; i < 1000000; ++i) second[static_cast<std::size_t>(i)] = i;
		test_merge("m = 0, n = 1000000", {}, second);
	}
	{
		// m = n = 1000000, all equal
		test_merge("m = n = 1000000, all equal",
			std::vector<int>(1000000, 42),
			std::vector<int>(1000000, 42));
	}
	{
		// m = 1000000, n = 1, the single element is the smallest
		std::vector<int> first(1000000);
		for (int i{0}; i < 1000000; ++i) first[static_cast<std::size_t>(i)] = i + 1;
		test_merge("m = 1000000, n = 1, the smallest", first, {0});
	}

	return 0;
}
```

Тесты проверены запуском. Заглушка даёт FAIL на 41 из 47 (остальные шесть — тривиальные случаи без слияния). Эталонное решение проходит 47 из 47, санитайзеры чистые. Слияние с пропущенным допереносом оставшихся элементов ловится 17 тестами, слияние слева направо без буфера — 28, ошибка на единицу в индексе — 12 (с падением). Вставка элементов со сдвигом не укладывается в минуту на больших блоках.

Пришлите своё решение, и я разберу его. Перед отправкой соберите с `-Wall -Wextra -Wconversion -Wsign-conversion` и запустите тесты.

---

work: 21:42 - 22:14


---

- nums1 :  {1, 2, 7, 0, 0, 0}, nums2: {2, 5, 6}
- nums1 :  {[1], 2, 7, 0, 0, 0}, nums2: {[2], 5, 6} => nums1:  {1, 2, 7, 0, 0, 0}, nums2: {2, 5, 6}
- nums1 :  {1, [2], 7, 0, 0, 0}, nums2: {[2], 5, 6} => nums1 :  {1, 2, 7, 0, 0, 0}, nums2: {2, 5, 6}
- nums1 :  {1, 2, [7], 0, 0, 0}, nums2: {[2], 5, 6} => nums1 :  {1, 2, 2, 0, 0, 0}, nums2: {[7], 5, 6}
- nums1 :  {1, 2, 2, [0], 0, 0}, nums2: {[7], 5, 6} => nums1 :  {1, 2, 2, 7, 0, 0}, nums2: {0, 5, 6}
- nums1 :  {1, 2, 2, [7], 0, 0}, nums2: {0, [5], 6} => nums1 :  {1, 2, 2, 5, 0, 0}, nums2: {0, 7, 6}
- nums1 :  {1, 2, 2, 5, [0], 0}, nums2: {0, [7], 6} => nums1 :  {1, 2, 2, 5, 7, 0}, nums2: {0, 0, 6}
- nums1 :  {1, 2, 2, 5, [7], 0}, nums2: {0, 0, [6]} => nums1 :  {1, 2, 2, 5, 6, 0}, nums2: {0, 0, 7}
- nums1 :  {1, 2, 2, 5, 6, [0]}, nums2: {0, 0, [7]} => nums1 :  {1, 2, 2, 5, 6, 7}, nums2: {0, 0, 0}

- nums1 :  {1, 2, [7], 0, 0, (0)}, nums2: {2, 5, [6]} => nums1 :  {1, 2, [7], 0, 0, (7)}, nums2: {2, 5, [6]}
- nums1 :  {1, [2], 7, 0, (0), 7}, nums2: {2, 5, [6]} => nums1 :  {1, [2], 7, 0, (6), 7}, nums2: {2, 5, [6]}
- nums1 :  {1, [2], 7, (0), 6, 7}, nums2: {2, [5], 6} => nums1 :  {1, [2], 7, (5), 6, 7}, nums2: {2, [5], 6}
- nums1 :  {1, [2], (7), 5, 6, 7}, nums2: {[2], 5, 6} => nums1 :  {1, [2], (2), 5, 6, 7}, nums2: {[2], 5, 6}
- nums1 :  {1, [2], 2, 5, 6, 7}, nums2: {[2], 5, 6}

---

```
	// int left{m - 1};
	// int right{n - 1};
	// int target{m + n - 1};
 //
 //    while (right >= 0) {
	//     if (left >= 0 && nums1[get_id(left)] > nums2[get_id(right)]) {
	// 	    nums1[get_id(target--)] = nums1[get_id(left--)];
	//     } else {
	// 	    nums1[get_id(target--)] = nums1[get_id(right--)];
	//     }
 //    }
 
 
 int left{m - 1};  
int right{n - 1};  
int target{m + n - 1};  
  
const auto get_id = [] (const int id) -> std::size_t {  
    return static_cast<std::size_t>(id);  
};  
  
while (right >= 0) {  
    if (left >= 0 && nums1[get_id(left)] > nums2[get_id(right)]) {  
       nums1[get_id(target--)] = nums1[get_id(left--)];  
    } else  {  
       nums1[get_id(target--)] = nums2[get_id(right--)];  
    }}
```


```cpp
#include <iostream>
#include <format>
#include <vector>
#include <string>
#include <algorithm>
#include <cstddef>

namespace {
	void merge_sorted(std::vector<int>& nums1,
					  int m,
					  const std::vector<int>& nums2,
					  int n) {

// nums1 :  {1, 2, [7], 0, 0, (0)}, nums2: {2, 5, [6]}
	// => nums1 :  {1, 2, [7], 0, 0, (7)}, nums2: {2, 5, [6]}
// nums1 :  {1, [2], 7, 0, (0), 7}, nums2: {2, 5, [6]}
	// => nums1 :  {1, [2], 7, 0, (6), 7}, nums2: {2, 5, [6]}
// nums1 :  {1, [2], 7, (0), 6, 7}, nums2: {2, [5], 6}
	// => nums1 :  {1, [2], 7, (5), 6, 7}, nums2: {2, [5], 6}
// nums1 :  {1, [2], (7), 5, 6, 7}, nums2: {[2], 5, 6}
	// => nums1 :  {1, [2], (2), 5, 6, 7}, nums2: {[2], 5, 6}
// nums1 :  {1, [2], 2, 5, 6, 7}, nums2: {[2], 5, 6}		

		int idx1{m - 1};
		int idx2{n - 1};
		int target_idx{m + n - 1};
		
		const auto get_id = [] (const int id) -> std::size_t {
			return static_cast<std::size_t>(id);
		};
		
		while (idx2 >= 0) {
			if (idx1 >= 0 && nums1[get_id(idx1)] > nums2[get_id(idx)]) {
				nums1[get_id(target_idx--)] = nums1[get_id(idx1--)];
			} else  {
				nums1[get_id(target_idx--)] = nums2[get_id(idx2--)];
			}
		}
	}

	void test_merge(const std::string& label,
					const std::vector<int>& first,
					const std::vector<int>& second) {
		const int m{static_cast<int>(first.size())};
		const int n{static_cast<int>(second.size())};

		std::vector<int> expected{first};
		expected.insert(expected.end(), second.begin(), second.end());
		std::ranges::sort(expected);

		std::vector<int> nums1{first};
		nums1.resize(first.size() + second.size(), 0);

		merge_sorted(nums1, m, second, n);

		std::cout << std::format("{}: {}\n",
			label,
			nums1 == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_merge("description example 1",
		{1, 2, 3},
		{2, 5, 6});
	test_merge("description example 2",
		{1},
		{});
	test_merge("description example 3",
		{},
		{1});
	test_merge("both empty",
		{},
		{});
	test_merge("single elements, in order",
		{1},
		{2});
	test_merge("single elements, swapped",
		{2},
		{1});
	test_merge("single equal elements",
		{5},
		{5});
	test_merge("second array is empty",
		{1, 2, 3, 4},
		{});
	test_merge("first array is empty",
		{},
		{1, 2, 3, 4});
	test_merge("all of the second are smaller",
		{5, 6, 7},
		{1, 2, 3});
	test_merge("all of the second are greater",
		{1, 2, 3},
		{5, 6, 7});
	test_merge("interleaved one by one",
		{1, 3, 5, 7},
		{2, 4, 6, 8});
	test_merge("interleaved, second starts",
		{2, 4, 6, 8},
		{1, 3, 5, 7});
	test_merge("second fits in the middle",
		{1, 2, 9, 10},
		{4, 5});
	test_merge("second goes to both ends",
		{5, 6, 7},
		{1, 9});
	test_merge("second is longer",
		{4},
		{1, 2, 3, 5, 6});
	test_merge("first is longer",
		{1, 2, 3, 5, 6},
		{4});
	test_merge("all equal values",
		{7, 7, 7},
		{7, 7});
	test_merge("duplicates across arrays",
		{1, 2, 2, 3},
		{2, 2, 4});
	test_merge("negative values",
		{-5, -3, 0},
		{-4, -1, 2});
	test_merge("all negative, second smaller",
		{-3, -2, -1},
		{-9, -8});
	test_merge("zero values mixed with placeholders",
		{0, 0, 1},
		{0, 2});
	test_merge("zeros in both",
		{0, 0, 0},
		{0, 0});
	test_merge("extreme values",
		{-1000000000, 0, 1000000000},
		{-1000000000, 1000000000});
	test_merge("second is a single element, smallest",
		{2, 3, 4},
		{1});
	test_merge("second is a single element, largest",
		{2, 3, 4},
		{9});
	test_merge("first is a single element, smallest",
		{1},
		{2, 3, 4});
	test_merge("first is a single element, largest",
		{9},
		{2, 3, 4});
	test_merge("runs alternate in blocks",
		{1, 2, 3, 10, 11, 12},
		{4, 5, 6, 13, 14, 15});
	test_merge("remaining of second after first is exhausted",
		{3, 4},
		{1, 2, 5, 6, 7});
	test_merge("remaining of first after second is exhausted",
		{1, 2, 5, 6, 7},
		{3, 4});
	test_merge("random 1, m=6, n=3",
		{-6, -4, -1, 2, 4, 6},
		{-6, -1, 5});
	test_merge("random 2, m=10, n=7",
		{-5, -4, -4, -4, -3, -1, 0, 1, 3, 5},
		{-5, -4, -4, -2, 0, 1, 4});
	test_merge("random 3, m=13, n=13",
		{-6, -6, -4, -4, -3, -3, -3, -3, -1, 1, 1, 2, 4},
		{-6, -5, -4, 0, 0, 1, 1, 1, 2, 2, 2, 5, 6});
	test_merge("random 4, m=6, n=0",
		{-4, -4, -2, 0, 4, 5},
		{});
	test_merge("random 5, m=0, n=14",
		{},
		{-6, -5, -5, -4, -3, -2, -1, -1, 0, 0, 2, 2, 2, 5});
	test_merge("random 6, m=13, n=7",
		{-4, -2, -1, -1, -1, 0, 0, 3, 3, 3, 3, 4, 5},
		{-1, 0, 1, 2, 3, 6, 6});
	test_merge("random 7, m=0, n=10",
		{},
		{-6, -6, -3, -3, -2, -1, -1, 0, 0, 6});
	test_merge("random 8, m=8, n=7",
		{-6, -5, -2, 2, 5, 5, 6, 6},
		{-6, -4, -3, -2, -1, 2, 5});

	{
		// m = n = 1000000, interleaved: first holds evens, second holds odds
		std::vector<int> first(1000000);
		std::vector<int> second(1000000);
		for (int i{0}; i < 1000000; ++i) {
			first[static_cast<std::size_t>(i)] = 2 * i;
			second[static_cast<std::size_t>(i)] = 2 * i + 1;
		}
		test_merge("m = n = 1000000, interleaved", first, second);
	}
	{
		// m = n = 1000000, all of the second are smaller (worst case for shifting)
		std::vector<int> first(1000000);
		std::vector<int> second(1000000);
		for (int i{0}; i < 1000000; ++i) {
			first[static_cast<std::size_t>(i)] = 1000000 + i;
			second[static_cast<std::size_t>(i)] = i;
		}
		test_merge("m = n = 1000000, all of the second are smaller", first, second);
	}
	{
		// m = n = 1000000, all of the second are greater
		std::vector<int> first(1000000);
		std::vector<int> second(1000000);
		for (int i{0}; i < 1000000; ++i) {
			first[static_cast<std::size_t>(i)] = i;
			second[static_cast<std::size_t>(i)] = 1000000 + i;
		}
		test_merge("m = n = 1000000, all of the second are greater", first, second);
	}
	{
		// m = n = 1000000, pseudo-random sorted values, with duplicates
		std::vector<int> first(1000000);
		std::vector<int> second(1000000);
		unsigned state{777};
		for (auto& x : first) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 8) % 2000001u) - 1000000;
		}
		for (auto& x : second) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 8) % 2000001u) - 1000000;
		}
		std::ranges::sort(first);
		std::ranges::sort(second);
		test_merge("m = n = 1000000, random with duplicates", first, second);
	}
	{
		// m = 1000000, n = 0
		std::vector<int> first(1000000);
		for (int i{0}; i < 1000000; ++i) first[static_cast<std::size_t>(i)] = i;
		test_merge("m = 1000000, n = 0", first, {});
	}
	{
		// m = 0, n = 1000000
		std::vector<int> second(1000000);
		for (int i{0}; i < 1000000; ++i) second[static_cast<std::size_t>(i)] = i;
		test_merge("m = 0, n = 1000000", {}, second);
	}
	{
		// m = n = 1000000, all equal
		test_merge("m = n = 1000000, all equal",
			std::vector<int>(1000000, 42),
			std::vector<int>(1000000, 42));
	}
	{
		// m = 1000000, n = 1, the single element is the smallest
		std::vector<int> first(1000000);
		for (int i{0}; i < 1000000; ++i) first[static_cast<std::size_t>(i)] = i + 1;
		test_merge("m = 1000000, n = 1, the smallest", first, {0});
	}

	return 0;
}
```