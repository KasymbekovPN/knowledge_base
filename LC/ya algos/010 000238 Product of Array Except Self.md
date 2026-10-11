## Задача: Product of Array Except Self (LeetCode 238, medium)

**Условие.** Дан массив целых чисел `nums`. Нужно вернуть массив `answer`, где `answer[i]` равен произведению всех элементов `nums`, кроме `nums[i]`.

Произведение любого префикса и любого суффикса массива гарантированно помещается в 32-битный `int`. Решение должно работать за O(n) **без операции деления**.

**Сигнатура:**

```cpp
std::vector<int> productExceptSelf(std::vector<int>& nums);
```

**Ограничения:**

- `2 <= n <= 10^5`
- `-30 <= nums[i] <= 30`

**Что обычно спрашивает интервьюер:**

1. Решение в лоб и его сложность.
2. Решение через общее произведение и деление. Почему оно не подходит? Что произойдёт при одном нуле и при двух нулях?
3. Как получить O(n) без деления? Что хранить для каждой позиции?
4. Можно ли обойтись O(1) дополнительной памяти (выходной массив не считается)?
5. Что поменяется, если ограничение на размер произведений снять? Как обнаружить переполнение?

**Целевая сложность:** O(n) по времени, O(1) дополнительной памяти без учёта ответа.


---

работа : 16:12 - 16:21, 13:12 - 

---

prefix idx postfix
? - not specified

- [1], 2, 3, 4 => (1) 0 24
- 1, [2], 3, 4 => 1 1 12
- 1, 2, [3], 4 => 2 2 4
- 1, 2, 3, [4] => 6 2 (1)

---


```cpp
#include <format>
#include <iostream>
#include <string>
#include <vector>

namespace {
	constexpr int MIN_SIZE{2};
	using Nums = std::vector<int>;

	Nums get_product_except_self(const Nums& nums) {
		const auto size{static_cast<int>(nums.size())};
		if (size < MIN_SIZE) return {};

		Nums result(size, 1);
		for (int i{1}; i < size; ++i) {
			result[i] = result[i-1] * nums[i-1];
		}
		
		int suff{1};
		for (int i{size - 1}; i >= 0; --i) {
			result[i] *= suff;
			suff *= nums[i];
		}

		return result;
	}

	void test_product_except_self(const std::string& label,
								  const Nums& nums,
								  const Nums& expected) {
		const auto result{get_product_except_self(nums)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_product_except_self("description example 1", {1, 2, 3, 4}, {24, 12, 8, 6});
	test_product_except_self("description example 2 (one zero)", {-1, 1, 0, -3, 3}, {0, 0, 9, 0, 0});
	test_product_except_self("min n", {3, 5}, {5, 3});
	test_product_except_self("min n with zero", {0, 7}, {7, 0});
	test_product_except_self("min n both zero", {0, 0}, {0, 0});
	test_product_except_self("all ones", {1, 1, 1, 1, 1}, {1, 1, 1, 1, 1});
	test_product_except_self("all equal", {2, 2, 2, 2}, {8, 8, 8, 8});
	test_product_except_self("one zero at start", {0, 2, 3, 4}, {24, 0, 0, 0});
	test_product_except_self("one zero at end", {2, 3, 4, 0}, {0, 0, 0, 24});
	test_product_except_self("one zero in middle", {2, 0, 3, 4}, {0, 24, 0, 0});
	test_product_except_self("two zeros", {0, 2, 0, 4}, {0, 0, 0, 0});
	test_product_except_self("all zeros", {0, 0, 0, 0}, {0, 0, 0, 0});
	test_product_except_self("all negative, even count", {-1, -2, -3, -4}, {-24, -12, -8, -6});
	test_product_except_self("all negative, odd count", {-1, -2, -3}, {6, 3, 2});
	test_product_except_self("mixed signs", {-2, 3, -4, 5, -1}, {60, -40, 30, -24, 120});
	test_product_except_self("with ones and minus ones", {1, -1, 1, -1, 1, -1, 1}, {-1, 1, -1, 1, -1, 1, -1});
	test_product_except_self("max abs values", {30, -30, 30, -30, 2}, {54000, -54000, 54000, -54000, 810000});
	test_product_except_self("random, n small", {0, -4, 1, 5, -4, -4, -3, -3, -4, 3, 3, 4}, {414720, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
	test_product_except_self("random, other", {-5, -4, -3, -1, -1, 1, -5, 1, -1, -1}, {-60, -75, -100, -300, -300, 300, -60, 300, -300, -300});
	test_product_except_self("random with zeros", {-1, -1, 1, 0, 0, 1, 1, 1, 2, 0, 2, 1, -1, -2}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
	{
		// n = 100000, all ones -> all ones
		test_product_except_self("n = 100000, all ones", Nums(100000, 1), Nums(100000, 1));
	}
	{
		// n = 100000, single 2 in the middle -> 2 everywhere except that position
		Nums nums(100000, 1);
		Nums expected(100000, 2);
		nums[50000] = 2;
		expected[50000] = 1;
		test_product_except_self("n = 100000, one two", nums, expected);
	}
	{
		// n = 100000, single zero -> zeros everywhere except that position
		Nums nums(100000, 1);
		Nums expected(100000, 0);
		nums[777] = 0;
		expected[777] = 1;
		test_product_except_self("n = 100000, one zero", nums, expected);
	}

	return 0;
}
```