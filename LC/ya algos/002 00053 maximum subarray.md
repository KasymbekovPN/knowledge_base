
# 53. Maximum Subarray (Medium)

Дан массив целых чисел `nums`. Нужно найти **непрерывный подмассив** (содержащий хотя бы один элемент) с **наибольшей суммой** и вернуть эту сумму.

Подмассив это последовательность подряд идущих элементов исходного массива, пропускать элементы нельзя.

**Ограничения:** `1 ≤ nums.length ≤ 10⁵`, `-10⁴ ≤ nums[i] ≤ 10⁴`.

**Примеры:**

```
Вход:  nums = [-2, 1, -3, 4, -1, 2, 1, -5, 4]
Выход: 6
Пояснение: подмассив [4, -1, 2, 1] имеет наибольшую сумму 6.
```

```
Вход:  nums = [5, 4, -1, 7, 8]
Выход: 23
Пояснение: подходит весь массив.
```

```
Вход:  nums = [-3, -1, -2]
Выход: -1
Пояснение: все числа отрицательные, подмассив должен быть непустым, поэтому берём наименьшее по модулю.
```

**Доп. вопрос из условия:** если решение за O(n) найдено, можно попробовать написать вариант методом «разделяй и властвуй».

Решайте по привычному протоколу: 2-3 примера и крайние случаи (один элемент, все отрицательные), подход и сложность вслух, затем код в блокноте и тесты, ориентир 20 минут. Пришлите код, разберу.

---

Работа:  12:00 - 12:22
Доработка: 12:25 - 12:28

---
	array, local_max, total_max

- [[-2], 1, -3, 4, -1, 2, 1, -5, 4] : -2, -2
- [[-2, 1], -3, 4, -1, 2, 1, -5, 4] : -1, -1
- [[-2, 1], -3, 4, -1, 2, 1, -5, 4] : -4 => [-2, 1, [-3], 4, -1, 2, 1, -5, 4] : -3, -2
- [-2, 1, [-3, 4], -1, 2, 1, -5, 4] : 1, 1 => [-2, 1, -3, [4], -1, 2, 1, -5, 4] : 4, 4
- [-2, 1, -3, [4, -1], 2, 1, -5, 4] : 3, 4
- [-2, 1, -3, [4, -1, 2], 1, -5, 4] : 5, 5
- [-2, 1, -3, [4, -1, 2, 1], -5, 4] : 6, 6
- [-2, 1, -3, [4, -1, 2, 1, -5], 4] : 1, 6
- [-2, 1, -3, [4, -1, 2, 1, -5, 4]] : 5, 6
- [-2, 1, -3, 4, -1, 2, 1, -5, [4]] : 4, 6

---

```cpp

#include <vector>
#include <iostream>
#include <format>
#include <string>
#include <algorithm>

namespace {
	int max_subarray(const std::vector<int>& input) {
		int best{input[0]};
		int best_ending_here{input[0]};
	
		for (int i{1}; i < static_cast<int>(input.size()); ++i) {
			const auto current_value{input[i]};
			best_ending_here = std::max(current_value, best_ending_here + current_value);
			best = std::max(best, best_ending_here);
		}
	
		return best;
	}
	
	void test_max_subarray(const std::string& label,
						   const std::vector<int>& input,
						   const int expected) {
		const auto result{max_subarray(input)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_max_subarray("long0", {-2, 1, -3, 4, -1, 2, 1, -5, 4}, 6);
	test_max_subarray("medium0", {5, 4, -1, 7, 8}, 23);
	test_max_subarray("short0", {-3, -1, -2}, -1);
	test_max_subarray("left border", {100, -1, -2, -3}, 100);
	test_max_subarray("right border", {-2, -1, -3, -4, -1, -2, -1, -5, 42}, 42);
	test_max_subarray("one", {13}, 13);

	return 0;
}

```




