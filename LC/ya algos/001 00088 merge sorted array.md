# 88. Merge Sorted Array (Easy)

Даны два массива целых чисел `nums1` и `nums2`, **отсортированные по неубыванию**, и два числа `m` и `n`: количество «настоящих» элементов в `nums1` и `nums2` соответственно.

Нужно **влить `nums2` в `nums1`** так, чтобы `nums1` стал одним отсортированным по неубыванию массивом. Функция ничего не возвращает, результат должен лежать в самом `nums1`.

Чтобы для этого хватило места, длина `nums1` равна `m + n`: первые `m` элементов это данные для слияния, а последние `n` элементов это нули-заглушки, которые нужно игнорировать. Длина `nums2` равна `n`.

**Ограничения:** `nums1.length == m + n`, `nums2.length == n`, `0 ≤ m, n ≤ 200`, `1 ≤ m + n ≤ 200`, значения от `-10⁹` до `10⁹`.

**Примеры:**

```
Вход:  nums1 = [1, 2, 3, 0, 0, 0], m = 3, nums2 = [2, 5, 6], n = 3
Выход: nums1 = [1, 2, 2, 3, 5, 6]
```

```
Вход:  nums1 = [0], m = 0, nums2 = [1], n = 1
Выход: nums1 = [1]
```

Во втором примере единственный ноль в `nums1` это заглушка (`m = 0`, значит настоящих элементов в `nums1` нет), а не значение для слияния.

---

- работа 12:32 - 12:45 + 16:51 - 17:07 + 17:10 - 17:21
- правка

---

- резервируем для nums1 -> n + m
- итерируем i -> [0, n + m]
- так же храним текущий nums2_idx (изначально == 0)
- сравниваем nums1[i] & nums2[nums2_idx]
	- nums1[i] < nums2[nums2_idx] - только ++i
	- nums1[i] > nums2[nums2_idx] - свапаем nums1[i] <-> nums2[nums2_idx]
- добавляем nums2 хвостом
- обрезаем по размеру (n + m)

---
---

- [[1], 2, 7, 0, 0, 0]  [[2], 5, 6]
- [1, [2], 7, 0, 0, 0]  [[2], 5, 6]
- [1, 2, [7], 0, 0, 0]  [[2], 5, 6] -> [1, 2, [2], 0, 0, 0]  [[7], 5, 6]
- [1, 2, 2, [0], 0, 0]  [[7], 5, 6] -> [1, 2, 2, [7], 0, 0]  [7, [5], 6]
- [1, 2, 2, [7], 0, 0]  [7, [5], 6] -> [1, 2, 2, [5], 0, 0]  [7, [7], 6]
- [1, 2, 2, 5, [0], 0]  [7, [7], 6] -> [1, 2, 2, 5, [7], 0]  [7, 7, [6]]
- [1, 2, 2, 5, [7], 0]  [7, 7, [6]] -> [1, 2, 2, 5, 6, [0]]  [7, 7, [7]]
- [1, 2, 2, 5, 6, [0]]  [7, 7, [7]] -> [1, 2, 2, 5, 6, [7]]  [7, 7, [7]]

---

```cpp
#include <iostream>
#include <format>
#include <vector>
#include <cstddef>
#include <utility>
#include <string>

namespace {
	void merge_sorted_array(std::vector<int>& nums1,
							const int m,
							std::vector<int>& nums2,
							const int n) {

		const auto state{(m == 0 ? 2 : 0) + (n == 0 ? 1 : 0)};	
		switch(state) {
			case 1: {
				nums1.erase(nums1.begin() + m, nums1.end());
				return;
			}
			case 2: {
				nums1 = nums2;
				return;
			}
			case 3: {
				nums1.clear();
				return;
			}
			default: break;
		}

		const std::size_t size{static_cast<std::size_t>(m + n)};
		for (std::size_t fill_count{static_cast<std::size_t>(m)}, nums1_idx{0}, nums2_idx{0};
			 nums1_idx < size;) {
			 
			if (fill_count == nums1_idx) {
				std::swap(nums1[nums1_idx], nums2[nums2_idx]);
				++nums2_idx;
				++fill_count;
			} else {
				if (nums1[nums1_idx] > nums2[nums2_idx]) {
					std::swap(nums1[nums1_idx], nums2[nums2_idx]);
				}
				++nums1_idx;
			}
		}
	}

	void test_merge_sorted_array(const std::string& label,
								 std::vector<int>& nums1,
								 const int m,
								 std::vector<int>& nums2,
								 const int n,
								 const std::vector<int>& expected) {
		merge_sorted_array(nums1, m, nums2, n);
		std::cout << std::format("{}: {}\n",
			label,
			nums1 == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	std::vector<int> nums1_0{1, 2, 3, 0, 0, 0};
	std::vector<int> nums2_0{2, 5, 6};
	std::vector<int> expected_0{1, 2, 2, 3, 5, 6};
	test_merge_sorted_array("first", nums1_0, 3, nums2_0, 3, expected_0);
	
	std::vector<int> nums1_1{0};
	std::vector<int> nums2_1{1};
	std::vector<int> expected_1{1};
	test_merge_sorted_array("second", nums1_1, 0, nums2_1, 1, expected_1);

	return 0;
}
```