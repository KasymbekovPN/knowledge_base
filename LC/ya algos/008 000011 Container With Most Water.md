## Задача: Container With Most Water

**Условие.** Дан массив неотрицательных целых `height` длины `n`. Число `height[i]` — высота вертикальной линии в точке `x = i`. Нужно выбрать две линии `i < j` так, чтобы вместе с осью X они образовывали ёмкость максимальной площади. Вернуть эту площадь. Ёмкость наклонять нельзя.

Площадь пары: `min(height[i], height[j]) * (j - i)`.

**Сигнатура:**

```cpp
int maxArea(const std::vector<int>& height);
```

**Формат ввода:** первая строка — `n`, вторая — `n` чисел через пробел.  
**Формат вывода:** одно число — максимальная площадь.

**Ограничения:**

- `2 <= n <= 10^5`
- `0 <= height[i] <= 10^4`

**Что обычно спрашивает интервьюер:**

1. Сначала решение «в лоб» и его сложность.
2. Можно ли быстрее, чем O(n²)?
3. Доказать корректность (почему не теряем оптимальную пару).
4. Нужен ли `long long`? Обоснуйте по ограничениям.
5. Что изменится, если массив нельзя хранить целиком, а он приходит потоком?

**Целевая сложность:** O(n) по времени, O(1) по памяти.

## Тестовые данные

| №   | Ввод                                                                                  | Ожидаемый вывод | Что проверяет                             |
| --- | ------------------------------------------------------------------------------------- | --------------- | ----------------------------------------- |
| 1   | `1 8 6 2 5 4 8 3 7`                                                                   | 49              | пример из условия                         |
| 2   | `1 1`                                                                                 | 1               | минимальный n                             |
| 3   | `0 0`                                                                                 | 0               | нулевые высоты                            |
| 4   | `1 2 3 4 5`                                                                           | 6               | возрастающие                              |
| 5   | `5 4 3 2 1`                                                                           | 6               | убывающие                                 |
| 6   | `7 7 7 7 7 7`                                                                         | 35              | все равны, ширина решает                  |
| 7   | `9 1 1 1 1 1 1 9`                                                                     | 63              | высокие края, мусор внутри                |
| 8   | `1 1 1 10 10 1 1 1`                                                                   | 10              | высокие стенки рядом, ширина 1            |
| 9   | `0 5 0 0 0 5 0`                                                                       | 20              | нули вокруг и между                       |
| 10  | `2 3 10 5 7 8 9`                                                                      | 36              | пик в середине не оптимален               |
| 11  | `14 17 14 14 16 18 6 5 16 15 20 19 5 3 14`                                            | 196             | случайный, n=15                           |
| 12  | `38 18 11 68 88 81 5 76 50 57 83 94 78 83 20 79 1 67 8 7 4 24 30 76 3 99 59 41 56 75` | 1875            | случайный, n=30                           |
| 13  | `n = 100000`, все элементы `10000`                                                    | 999990000       | максимум; проверка переполнения и времени |

Тест 13 нужно генерировать кодом: `std::vector<int>(100000, 10000)`. O(n²) на нём не уложится в лимит, а O(n) отработает мгновенно.

Все ожидаемые ответы я посчитал перебором O(n²), независимо от идеи с указателями. Файл с тестами в формате «ввод/вывод»: `/home/claude/lc11/interview_tests.txt`.

Пришлите свой вариант, и я проверю его на этих тестах. Если застрянете, дам подсказку. Следующей по плану идёт Trapping Rain Water (LC42), её тоже оформлю так.

---

Работа: 13:40 - 13:57


---
[] - begin
() - end

- [1],8, 6, 2, 5, 4, 8, 3, [7] => 8
- 1,[8], 6, 2, 5, 4, 8, 3, [7] => 49
- 1,[8], 6, 2, 5, 4, 8, [3], 7=> 18

--- 

```cpp
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <format>

namespace {
	constexpr int MIN_LEN{2};

	int get_most_water_value(const std::vector<int>& heights) {
		const auto size{static_cast<int>(heights.size())};
		if (size < MIN_LEN) return 0;
		
		int best{0};
		int left{0};
		int right{size - 1};
		while (left < right) {
			const auto left_value{heights[left]};
			const auto right_value{heights[right]};
			best = std::max(
				best,
				std::min(left_value, right_value) * (right - left)
			);
			
			if (left_value < right_value) ++left;
			else --right;
		}
		
		return best;
	}
	
	void test_most_water_value(const std::string& label,
							   const std::vector<int>& heights,
							   const int expected) {
		const auto result{get_most_water_value(heights)};
		std::cout << std::format("{}: {}\n",
			label,
			expected = result ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_most_water_value("description example", {1, 8, 6, 2, 5, 4, 8, 3, 7}, 49);
	test_most_water_value("min n", {1, 1}, 1);
	test_most_water_value("zero", {0, 0}, 0);
	test_most_water_value("one", {5}, 0);
	test_most_water_value("empty", {}, 0);
	test_most_water_value("increasing", {1, 2, 3, 4, 5}, 6);
	test_most_water_value("all eq", {7, 7, 7, 7, 7, 7}, 35);
	test_most_water_value("high border", {9, 1, 1, 1, 1, 1, 1, 9}, 63);
	test_most_water_value("high in mid", {1, 1, 1, 10, 10, 1, 1, 1}, 10);
	test_most_water_value("zero around", {0, 5, 0, 0, 0, 5, 0}, 20);
	test_most_water_value("peak in mid", {2, 3, 10, 5, 7, 8, 9}, 36);
	test_most_water_value(
		"random, n=15",
		{14, 17, 14, 14, 16, 18, 6, 5, 16, 15, 20, 19, 5, 3, 14},
		196);
	test_most_water_value(
		"random, n=30",
		{
			38, 18, 11, 68, 88, 81, 5, 76, 50, 57,
			83, 94, 78, 83, 20, 79, 1, 67, 8, 7,
			4, 24, 30, 76, 3, 99, 59, 41, 56, 75
		},
		1875);
	test_most_water_value("n = 100000", std::vector<int>(100000, 10000), 999990000);

	return 0;
}

```