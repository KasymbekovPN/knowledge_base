

# 283. Move Zeroes (Easy)

Дан массив целых чисел `nums`. Нужно **переместить все нули в конец** массива, **сохранив относительный порядок** остальных (ненулевых) элементов. Менять массив нужно **на месте**, без создания копии. Функция ничего не возвращает.

**Ограничения:** `1 ≤ nums.length ≤ 10⁴`, значения элементов помещаются в `int`.

**Примеры:**

```
Вход:  nums = [0, 1, 0, 3, 12]
Выход: nums = [1, 3, 12, 0, 0]
```

```
Вход:  nums = [0]
Выход: nums = [0]
```

---

- работа - 11:58 - 12:15
- правки - 12:16 - 12:23 + 12:25 - 12:27

---

# Описание решения

- 0 [0, 1, 0, 3, 12]
- 1 [1, 0, 0, 3, 12]
- 2 [1, 0, 0, 3, 12]

```cpp
#include <cstddef>
#include <vector>
#include <utility>
#include <iostream>
#include <format>
#include <string>

namespace {
	void zero_moves(std::vector<int>& vec) {
		std::size_t idx{0};
		for (std::size_t i{0}; i < vec.size(); ++i) {
			if (vec[i] != 0) {
				if (idx != i) std::swap(vec[idx], vec[i]);
				idx++;
			}
		}
	}
	
	void test(const std::string& label,
			  std::vector<int> vec,
			  const std::vector<int>& expected) {
		zero_moves(vec);
		std::cout << std::format("{} {}\n", 
			label,
			vec == expected ? "OK" : "FAIL"
		) << std::flush;
	}
}

int main() {	
	test("mixed", {0, 1, 0, 3, 12}, {1, 3, 12, 0, 0});
	test("single", {0}, {0});
	test("no zero", {1, 2, 3}, {1, 2, 3});
	test("zeros", {0, 0, 0}, {0, 0, 0});
	test("tail", {1, 0}, {1, 0});
	test("lead", {1, 0, 2}, {1, 2, 0});
	test("empty", {}, {});

	return 0;
}

```
