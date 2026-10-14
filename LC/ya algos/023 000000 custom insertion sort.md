
## Блок 4 (сортировки), задача 1: 75. Sort Colors

**Условие.** Дан массив `nums` из `n` элементов, каждый равен 0, 1 или 2 (три «цвета»). Нужно отсортировать массив **на месте** по возрастанию, чтобы одинаковые значения шли подряд в порядке 0, 1, 2.

Нельзя использовать библиотечные сортировки (`std::sort`, `std::stable_sort` и так далее).

**Сигнатура.**

```cpp
void sort_colors(std::vector<int>& nums);
```

**Ограничения.**

- 1 ≤ n ≤ 10⁶
- `nums[i]` ∈ {0, 1, 2}

**Целевая сложность.** По времени O(n), по дополнительной памяти O(1).

**Вопросы интервьюера** (вслух, в процессе):

1. Какое самое простое решение вы видите? Какова его сложность по времени и по памяти?
2. Можно ли обойтись без дополнительных структур и получить O(n)? Сколько проходов по массиву понадобится?
3. Можно ли за один проход? Какой инвариант у вас получится для разных частей массива?
4. Что должно происходить с указателями после каждого обмена? Почему именно так?
5. Как решение обобщить на k цветов? Что в этом случае поменяется по сложности?

Сегодняшний час по плану: устный минимум по сортировкам (quick/merge/heap, устойчивость, `std::sort`, `stable_sort`, `nth_element`) плюс эта задача.

**Шаблон с тестами.**

```cpp
namespace {
	void sort_colors([[maybe_unused]] std::vector<int>& nums) {
	}

	void test_sort_colors(const std::string& label,
						  std::vector<int> nums) {
		std::vector<int> expected{nums};
		std::ranges::sort(expected);

		sort_colors(nums);

		std::cout << std::format("{}: {}\n",
			label,
			nums == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_sort_colors("description example 1",
		{2, 0, 2, 1, 1, 0});
	test_sort_colors("description example 2",
		{2, 0, 1});
	test_sort_colors("single element 0",
		{0});
	test_sort_colors("single element 1",
		{1});
	test_sort_colors("single element 2",
		{2});
	test_sort_colors("two elements, 2 and 0",
		{2, 0});
	test_sort_colors("two elements, 0 and 2",
		{0, 2});
	test_sort_colors("two elements, 1 and 0",
		{1, 0});
	test_sort_colors("two elements, 2 and 1",
		{2, 1});
	test_sort_colors("two elements, already sorted",
		{0, 1});
	test_sort_colors("two equal elements",
		{1, 1});
	test_sort_colors("all zeros",
		{0, 0, 0, 0, 0});
	test_sort_colors("all ones",
		{1, 1, 1, 1, 1});
	test_sort_colors("all twos",
		{2, 2, 2, 2, 2});
	test_sort_colors("already sorted",
		{0, 0, 1, 1, 2, 2});
	test_sort_colors("reverse sorted",
		{2, 2, 1, 1, 0, 0});
	test_sort_colors("only 0 and 2, alternating",
		{0, 2, 0, 2, 0, 2});
	test_sort_colors("only 0 and 1",
		{1, 0, 1, 1, 0, 0, 1});
	test_sort_colors("only 1 and 2",
		{2, 1, 2, 2, 1, 1, 2});
	test_sort_colors("no ones",
		{2, 0, 0, 2, 2, 0});
	test_sort_colors("swap with the right edge brings a 2",
		{2, 2, 0});
	test_sort_colors("swap with the right edge brings a 0",
		{1, 2, 0, 0});
	test_sort_colors("swap with the right edge brings a 1",
		{2, 1, 0});
	test_sort_colors("0 1 2 repeated",
		{0, 1, 2, 0, 1, 2, 0, 1, 2});
	test_sort_colors("2 1 0 repeated",
		{2, 1, 0, 2, 1, 0, 2, 1, 0});
	test_sort_colors("ones at both ends",
		{1, 2, 0, 2, 0, 1});
	test_sort_colors("zeros at the end",
		{2, 2, 2, 1, 1, 0, 0, 0});
	test_sort_colors("twos at the start, zeros in the middle",
		{2, 2, 0, 0, 1, 1});
	test_sort_colors("one zero among twos",
		{2, 2, 2, 2, 0, 2, 2});
	test_sort_colors("one two among zeros",
		{0, 0, 0, 2, 0, 0, 0});
	test_sort_colors("one one among zeros and twos",
		{0, 2, 0, 1, 2, 0, 2});
	test_sort_colors("random 1, n=22",
		{2, 1, 1, 0, 2, 2, 2, 2, 1, 2, 2, 2, 1, 0, 2, 1, 0, 1, 1, 1, 2, 2});
	test_sort_colors("random 2, n=19",
		{2, 2, 1, 1, 1, 0, 1, 2, 2, 2, 1, 2, 0, 0, 0, 0, 1, 1, 2});
	test_sort_colors("random 3, n=13",
		{1, 0, 1, 2, 2, 0, 0, 0, 1, 1, 0, 0, 0});
	test_sort_colors("random 4, n=21",
		{0, 1, 1, 2, 2, 0, 2, 1, 1, 1, 2, 2, 0, 0, 1, 2, 2, 2, 0, 0, 1});
	test_sort_colors("random 5, n=18",
		{1, 1, 0, 2, 0, 2, 2, 1, 0, 1, 2, 2, 1, 0, 1, 1, 1, 1});
	test_sort_colors("random 6, n=30",
		{1, 2, 0, 1, 1, 1, 2, 2, 1, 1, 0, 1, 1, 1, 1, 0, 2, 2, 1, 1, 2, 1, 1, 2, 0, 1, 2, 0, 2, 0});
	test_sort_colors("random 7, n=20",
		{2, 2, 2, 1, 0, 1, 1, 0, 0, 2, 2, 0, 0, 1, 1, 1, 2, 1, 2, 0});
	test_sort_colors("random 8, n=8",
		{1, 1, 0, 1, 0, 0, 1, 0});

	{
		// n = 1000000, pseudo-random values
		std::vector<int> nums(1000000);
		unsigned state{12345};
		for (auto& x : nums) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 16) % 3);
		}
		test_sort_colors("n = 1000000, random", nums);
	}
	{
		// n = 1000000, all 2 then all 1 then all 0 (worst case for quadratic sorts)
		std::vector<int> nums;
		nums.insert(nums.end(), 333334, 2);
		nums.insert(nums.end(), 333333, 1);
		nums.insert(nums.end(), 333333, 0);
		test_sort_colors("n = 1000000, reverse sorted blocks", nums);
	}
	{
		// n = 1000000, 0 1 2 0 1 2 ...
		std::vector<int> nums(1000000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = static_cast<int>(i % 3);
		test_sort_colors("n = 1000000, 0 1 2 repeated", nums);
	}
	{
		// n = 1000000, 2 1 0 2 1 0 ...
		std::vector<int> nums(1000000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = static_cast<int>(2 - i % 3);
		test_sort_colors("n = 1000000, 2 1 0 repeated", nums);
	}
	{
		// n = 1000000, all 1
		test_sort_colors("n = 1000000, all ones", std::vector<int>(1000000, 1));
	}
	{
		// n = 1000000, only 0 and 2, alternating
		std::vector<int> nums(1000000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = (i % 2 == 0) ? 0 : 2;
		test_sort_colors("n = 1000000, only 0 and 2, alternating", nums);
	}
	{
		// n = 1000000, a single 0 at the end of the twos
		std::vector<int> nums(1000000, 2);
		nums.back() = 0;
		test_sort_colors("n = 1000000, twos with one zero at the end", nums);
	}
	{
		// n = 1000000, a single 2 at the start of the zeros
		std::vector<int> nums(1000000, 0);
		nums.front() = 2;
		test_sort_colors("n = 1000000, zeros with one two at the start", nums);
	}

	return 0;
}
```

Тесты проверены запуском: заглушка даёт FAIL на 36 из 47 тестов, эталонное решение проходит 47 из 47 за 0,15 с, санитайзеры чистые. Квадратичная сортировка на больших блоках не укладывается в минуту. Однопроходный вариант с ошибкой в указателях ловится малыми тестами.

Пришлите своё решение, и я разберу его. Напомню про привычку: собрать с `-Wall -Wextra -Wconversion -Wsign-conversion` и запустить до отправки.

---

work: 22:30 -

---

- {2, 0, 2, 1, 1, 0}
- {[2, 0], 2, 1, (1, 0)} => {[0, 2], 2, 1, [0, 1]}
- {0, [2, 2], (1, 0), 1} => {0, [2, 2], (0, 1), 1}
- {0, 2, [2, 0], 1, 1} => {0, 2, [0, 2], 1, 1} => {0, 2, (0, 2), 1, 1} => {0, 2, (0, 2), 1, 1}
- {0, (2, 0), [2, 1], 1}  = > {0, (0, 2), [1, 2], 1}
- {(0, 0), 2, 1, [2, 1]} => {(0, 0), 2, 1, [1, 2]}

- {2, 0, 2, 1, 1, 0}
- {2, [0], 2, 1, 1, 0}

---


```cpp
#include <iostream>
#include <format>
#include <vector>
#include <string>
#include <algorithm>
#include <cstddef>

namespace {
	void sort_colons_ins(std::vector<int>& nums) {
		const std::size_t size{nums.size()};
		if (size == 0) return;
		
		for (std::size_t i{1}; i < size; ++i) {
			const auto current_value{nums[i]};
			auto j{i};
			
			while (j > 0 && nums[j] > current_value) {
				nums[j+1] = nums[j];
				--j;
			}
			
			nums[j+1] = current_value;
		}
    }
    
    void sort_colons(std::vector<int>& nums) {
	    std::size_t idx_0{0};
	    std::size_t idx_1{0};
	    std::size_t idx_2{nums.size()};
	    
	    while (idx_1 < idx_2) {
		    if (nums[idx_1] == 0) {
			    std::swap(nums[idx_0++], nums[idx_1++]);
		    } else if (nums[idx_1] == 1) {
			    ++idx_1;
		    } else {
			    std::swap(nums[idx_1], nums[--idx_2]);
		    }
	    }
    }

	void test_sort_colors(const std::string& label,
						  std::vector<int> nums) {
		auto expected{nums};
		std::ranges::sort(expected);

		auto num_insertion{nums};
		sort_colons_ins(nums_insertion);
		sort_colons(nums);

		std::cout << std::format("{}: insertion: {}, O(n): {}\n",
			label,
			nums_insertion == expected ? "OK" : "FAIL",
			nums == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_sort_colors("description example 1",
		{2, 0, 2, 1, 1, 0});
	test_sort_colors("description example 2",
		{2, 0, 1});
	test_sort_colors("single element 0",
		{0});
	test_sort_colors("single element 1",
		{1});
	test_sort_colors("single element 2",
		{2});
	test_sort_colors("two elements, 2 and 0",
		{2, 0});
	test_sort_colors("two elements, 0 and 2",
		{0, 2});
	test_sort_colors("two elements, 1 and 0",
		{1, 0});
	test_sort_colors("two elements, 2 and 1",
		{2, 1});
	test_sort_colors("two elements, already sorted",
		{0, 1});
	test_sort_colors("two equal elements",
		{1, 1});
	test_sort_colors("all zeros",
		{0, 0, 0, 0, 0});
	test_sort_colors("all ones",
		{1, 1, 1, 1, 1});
	test_sort_colors("all twos",
		{2, 2, 2, 2, 2});
	test_sort_colors("already sorted",
		{0, 0, 1, 1, 2, 2});
	test_sort_colors("reverse sorted",
		{2, 2, 1, 1, 0, 0});
	test_sort_colors("only 0 and 2, alternating",
		{0, 2, 0, 2, 0, 2});
	test_sort_colors("only 0 and 1",
		{1, 0, 1, 1, 0, 0, 1});
	test_sort_colors("only 1 and 2",
		{2, 1, 2, 2, 1, 1, 2});
	test_sort_colors("no ones",
		{2, 0, 0, 2, 2, 0});
	test_sort_colors("swap with the right edge brings a 2",
		{2, 2, 0});
	test_sort_colors("swap with the right edge brings a 0",
		{1, 2, 0, 0});
	test_sort_colors("swap with the right edge brings a 1",
		{2, 1, 0});
	test_sort_colors("0 1 2 repeated",
		{0, 1, 2, 0, 1, 2, 0, 1, 2});
	test_sort_colors("2 1 0 repeated",
		{2, 1, 0, 2, 1, 0, 2, 1, 0});
	test_sort_colors("ones at both ends",
		{1, 2, 0, 2, 0, 1});
	test_sort_colors("zeros at the end",
		{2, 2, 2, 1, 1, 0, 0, 0});
	test_sort_colors("twos at the start, zeros in the middle",
		{2, 2, 0, 0, 1, 1});
	test_sort_colors("one zero among twos",
		{2, 2, 2, 2, 0, 2, 2});
	test_sort_colors("one two among zeros",
		{0, 0, 0, 2, 0, 0, 0});
	test_sort_colors("one one among zeros and twos",
		{0, 2, 0, 1, 2, 0, 2});
	test_sort_colors("random 1, n=22",
		{2, 1, 1, 0, 2, 2, 2, 2, 1, 2, 2, 2, 1, 0, 2, 1, 0, 1, 1, 1, 2, 2});
	test_sort_colors("random 2, n=19",
		{2, 2, 1, 1, 1, 0, 1, 2, 2, 2, 1, 2, 0, 0, 0, 0, 1, 1, 2});
	test_sort_colors("random 3, n=13",
		{1, 0, 1, 2, 2, 0, 0, 0, 1, 1, 0, 0, 0});
	test_sort_colors("random 4, n=21",
		{0, 1, 1, 2, 2, 0, 2, 1, 1, 1, 2, 2, 0, 0, 1, 2, 2, 2, 0, 0, 1});
	test_sort_colors("random 5, n=18",
		{1, 1, 0, 2, 0, 2, 2, 1, 0, 1, 2, 2, 1, 0, 1, 1, 1, 1});
	test_sort_colors("random 6, n=30",
		{1, 2, 0, 1, 1, 1, 2, 2, 1, 1, 0, 1, 1, 1, 1, 0, 2, 2, 1, 1, 2, 1, 1, 2, 0, 1, 2, 0, 2, 0});
	test_sort_colors("random 7, n=20",
		{2, 2, 2, 1, 0, 1, 1, 0, 0, 2, 2, 0, 0, 1, 1, 1, 2, 1, 2, 0});
	test_sort_colors("random 8, n=8",
		{1, 1, 0, 1, 0, 0, 1, 0});

	{
		// n = 1000000, pseudo-random values
		std::vector<int> nums(1000000);
		unsigned state{12345};
		for (auto& x : nums) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 16) % 3);
		}
		test_sort_colors("n = 1000000, random", nums);
	}
	{
		// n = 1000000, all 2 then all 1 then all 0 (worst case for quadratic sorts)
		std::vector<int> nums;
		nums.insert(nums.end(), 333334, 2);
		nums.insert(nums.end(), 333333, 1);
		nums.insert(nums.end(), 333333, 0);
		test_sort_colors("n = 1000000, reverse sorted blocks", nums);
	}
	{
		// n = 1000000, 0 1 2 0 1 2 ...
		std::vector<int> nums(1000000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = static_cast<int>(i % 3);
		test_sort_colors("n = 1000000, 0 1 2 repeated", nums);
	}
	{
		// n = 1000000, 2 1 0 2 1 0 ...
		std::vector<int> nums(1000000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = static_cast<int>(2 - i % 3);
		test_sort_colors("n = 1000000, 2 1 0 repeated", nums);
	}
	{
		// n = 1000000, all 1
		test_sort_colors("n = 1000000, all ones", std::vector<int>(1000000, 1));
	}
	{
		// n = 1000000, only 0 and 2, alternating
		std::vector<int> nums(1000000);
		for (std::size_t i{0}; i < nums.size(); ++i) nums[i] = (i % 2 == 0) ? 0 : 2;
		test_sort_colors("n = 1000000, only 0 and 2, alternating", nums);
	}
	{
		// n = 1000000, a single 0 at the end of the twos
		std::vector<int> nums(1000000, 2);
		nums.back() = 0;
		test_sort_colors("n = 1000000, twos with one zero at the end", nums);
	}
	{
		// n = 1000000, a single 2 at the start of the zeros
		std::vector<int> nums(1000000, 0);
		nums.front() = 2;
		test_sort_colors("n = 1000000, zeros with one two at the start", nums);
	}

	return 0;
}
```