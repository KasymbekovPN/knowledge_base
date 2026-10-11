
# 209. Minimum Size Subarray Sum (Medium)

Дан массив **положительных** целых чисел `nums` и положительное целое число `target`. Нужно найти **минимальную длину непрерывного подмассива**, сумма элементов которого **больше или равна `target`**, и вернуть эту длину. Если такого подмассива нет, нужно вернуть `0`.

**Ограничения:** `1 ≤ target ≤ 10⁹`, `1 ≤ nums.length ≤ 10⁵`, `1 ≤ nums[i] ≤ 10⁴`.

**Примеры:**

```
Вход:  target = 7, nums = [2, 3, 1, 2, 4, 3]
Выход: 2
Пояснение: подмассив [4, 3] имеет сумму 7 и минимальную длину среди подходящих.
```

```
Вход:  target = 4, nums = [1, 4, 4]
Выход: 1
Пояснение: достаточно одного элемента 4.
```

```
Вход:  target = 11, nums = [1, 1, 1, 1, 1, 1, 1, 1]
Выход: 0
Пояснение: сумма всего массива равна 8, до 11 не дотягивает.
```

**Дополнительный вопрос из условия:** если решение за O(n) найдено, попробуйте написать другое решение со сложностью O(n log n).

Ориентир по времени: 30 минут. Начните с примеров и крайних случаев: `target` больше суммы всего массива, один элемент уже не меньше `target`, подходит ровно весь массив, одиночный элемент, `target` равен сумме всего массива. Назовите подход и сложность вслух: сначала простой вариант и его цену, затем оптимизацию. Затем код в блокноте с тестами. Пришлите код, разберу.

---

17:49 - 18:24

---

# `target` больше суммы всего массива

# один элемент уже не меньше `target`

# подходит ровно весь массив

# одиночный элемент

# `target` равен сумме всего массива

# 2, 3, 1, 2, 4, 3 | target 7

- ([2]), 3, 1, 2, 4, 3 => 2 [0, 0]
- [2], (3), 1, 2, 4, 3 => 5 [0, 1]
- [2], 3, (1), 2, 4, 3 => 6 [0, 2]
- [2], 3, 1, (2), 4, 3 => 8 [0, 3] => 2, [3], 1, (2), 4, 3 => 6 [1, 4]
- 2, [3], 1, 2, (4), 3 => 10 [1, 5] => 2, 3, [1], 2, (4), 3 => 7 [2, 4] => 2, 3, 1, [2], (4), 3 => 6 [3, 4]
- 2, 3, 1, [2], 4, [3] => 9 [3, 5] => 2, 3, 1, 2, [4], [3] => 7 [4, 5]

---

```cpp
#include <vector>
#include <algorithm>
#include <iostream>
#include <format>
#include <string>
#include <climits>
#include <cstddef>

namespace {

	int get_min_size_sub(const std::vector<int>& nums, const int target) {
		int sum{0};
		int best{INT_MAX};
		for (std::size_t left{0}, right{0}; right < nums.size(); ++right) {
			sum += nums[right];
			while (sum >= target) {
				best = std::min(best, static_cast<int>(right - left + 1));
				sum -= nums[left++];
			}
		}
	
		return best == INT_MAX ? 0 : best;
	}
	
	int get_min_size_sub_pref(const std::vector<int>& nums, const int target) {
		const std::size_t size{nums.size()};
		std::vector<int> pref(size + 1, 0);
		
		for (std::size_t i{0}; i < size; ++i) pref[i+1] = pref[i] + nums[i];
		
		int best{INT_MAX};
		
		for (std::size_t right{1}; right <= size; ++right) {
			auto need{pref[right] - target};
			auto it{std::upper_bound(pref.begin(), pref.begin() + right, need)};
			
			if (it != pref.begin() && it != pref.end()) {
				const auto dis{std::distance(pref.begin(), it - 1)};
				best = std::min(best, right - dis);
			}
		}
		
		return best == INT_MAX ? 0 : best;
	}
	
	void test_get_min_size_sub(const std::string& label,
							   const std::vector<int>& nums,
							   const int target,
							   const int expected) {
		const auto result{get_min_size_sub(nums, target)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_get_min_size_sub("on end", {2, 3, 1, 2, 4, 3}, 7, 2);
	test_get_min_size_sub("one match", {1, 4, 4}, 4, 1);
	test_get_min_size_sub("big target", {1, 1, 1, 1, 1, 1, 1, 1}, 11, 0);
	test_get_min_size_sub("total sum", {1, 2, 3}, 6, 3);
	test_get_min_size_sub("any match", {4, 4, 4}, 4, 1);

	return 0;
}

```


