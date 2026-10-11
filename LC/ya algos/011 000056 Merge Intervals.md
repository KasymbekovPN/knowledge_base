## Задача: Merge Intervals (LeetCode 56, medium)

**Условие.** Дан массив отрезков `intervals`, где `intervals[i] = [start_i, end_i]`. Нужно объединить все пересекающиеся отрезки и вернуть массив непересекающихся отрезков, которые покрывают ровно те же точки, что и входные. Отрезки замкнутые: `[1, 4]` и `[4, 5]` пересекаются в точке 4 и объединяются в `[1, 5]`. Отрезки `[1, 2]` и `[3, 4]` не пересекаются и остаются раздельными.

**Сигнатура:**

```cpp
std::vector<std::vector<int>> merge(std::vector<std::vector<int>>& intervals);
```

**Ограничения:**

- `1 <= intervals.size() <= 10^4`
- `intervals[i].size() == 2`
- `0 <= start_i <= end_i <= 10^4`

Входной массив не отсортирован. Порядок отрезков в ответе не важен: тестовая обвязка сама сортирует результат перед сравнением.

**Что обычно спрашивает интервьюер:**

1. Решение в лоб и его сложность. Как объединять, если отрезков два, и как, если их много?
2. Что нужно сделать с массивом, чтобы объединение стало одним проходом? Почему именно сортировка по началу?
3. Условие слияния: строго `<` или нестрого `<=`? Что в условии про касание в одной точке?
4. Как обновлять правую границу при вложенных отрезках (`[1, 10]` и `[2, 3]`)?
5. Можно ли быстрее, чем O(n log n), учитывая `0 <= x <= 10^4`? Что делать, если координаты большие?
6. Можно ли менять входной массив? Что с дополнительной памятью?

**Целевая сложность:** O(n log n) по времени (сортировка), O(n) п о памяти под ответ.

## Шаблон `main`

Нужные заголовки: `<algorithm>`, `<format>`, `<iostream>`, `<string>`, `<vector>`.

Решения в шаблоне нет, рабочий метод пустой и возвращает `{}`. Ожидаемые значения посчитаны независимым способом: покрытием точек на удвоенных координатах, а не сортировкой. Так касание в одной точке объединяется, а соседние целые числа остаются раздельными. Шаблон компилируется без предупреждений: заглушка даёт 26 `FAIL`, эталонное решение проходит все 26 тестов под `-fsanitize=address,undefined`.

```cpp
namespace {
	using Intervals = std::vector<std::vector<int>>;

	Intervals get_merged_intervals([[maybe_unused]] const Intervals& intervals) {
		return {};
	}

	void test_merge_intervals(const std::string& label,
							  const Intervals& intervals,
							  const Intervals& expected) {
		auto result{get_merged_intervals(intervals)};
		std::ranges::sort(result);   // порядок в ответе не важен
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_merge_intervals("description example 1",
		{{1, 3}, {2, 6}, {8, 10}, {15, 18}},
		{{1, 6}, {8, 10}, {15, 18}});
	test_merge_intervals("description example 2 (touching)",
		{{1, 4}, {4, 5}},
		{{1, 5}});
	test_merge_intervals("single interval",
		{{5, 7}},
		{{5, 7}});
	test_merge_intervals("single point interval",
		{{3, 3}},
		{{3, 3}});
	test_merge_intervals("two disjoint, sorted",
		{{1, 2}, {4, 5}},
		{{1, 2}, {4, 5}});
	test_merge_intervals("two disjoint, unsorted",
		{{4, 5}, {1, 2}},
		{{1, 2}, {4, 5}});
	test_merge_intervals("adjacent integers do not merge",
		{{1, 2}, {3, 4}},
		{{1, 2}, {3, 4}});
	test_merge_intervals("identical intervals",
		{{2, 5}, {2, 5}, {2, 5}},
		{{2, 5}});
	test_merge_intervals("nested",
		{{1, 10}, {2, 3}, {4, 5}, {6, 9}},
		{{1, 10}});
	test_merge_intervals("nested, inner first",
		{{2, 3}, {1, 10}},
		{{1, 10}});
	test_merge_intervals("touching chain",
		{{1, 2}, {2, 3}, {3, 4}, {4, 5}},
		{{1, 5}});
	test_merge_intervals("all merge, unsorted",
		{{6, 9}, {1, 4}, {3, 7}, {8, 12}},
		{{1, 12}});
	test_merge_intervals("point inside interval",
		{{1, 5}, {3, 3}},
		{{1, 5}});
	test_merge_intervals("points only",
		{{7, 7}, {2, 2}, {7, 7}, {5, 5}},
		{{2, 2}, {5, 5}, {7, 7}});
	test_merge_intervals("same start, different end",
		{{1, 4}, {1, 2}, {1, 9}},
		{{1, 9}});
	test_merge_intervals("same end, different start",
		{{1, 9}, {5, 9}, {3, 9}},
		{{1, 9}});
	test_merge_intervals("zero start",
		{{0, 0}, {0, 1}, {2, 3}},
		{{0, 1}, {2, 3}});
	test_merge_intervals("max value",
		{{10000, 10000}, {9999, 10000}, {0, 1}},
		{{0, 1}, {9999, 10000}});
	test_merge_intervals("late long interval absorbs earlier",
		{{1, 2}, {3, 4}, {5, 6}, {0, 10}},
		{{0, 10}});
	test_merge_intervals("mixed groups",
		{{9, 11}, {1, 3}, {12, 14}, {2, 4}, {5, 5}, {10, 13}, {20, 22}, {21, 25}},
		{{1, 4}, {5, 5}, {9, 14}, {20, 25}});
	test_merge_intervals("random 1, n=14",
		{{35, 35}, {30, 35}, {33, 35}, {34, 37}, {14, 14}, {23, 27}, {4, 6}, {16, 20}, {13, 13}, {36, 37}, {3, 7}, {35, 36}, {24, 29}, {1, 2}},
		{{1, 2}, {3, 7}, {13, 13}, {14, 14}, {16, 20}, {23, 29}, {30, 37}});
	test_merge_intervals("random 2, n=11",
		{{15, 16}, {6, 10}, {12, 16}, {13, 16}, {11, 16}, {24, 25}, {4, 10}, {13, 17}, {11, 12}, {11, 12}, {35, 38}},
		{{4, 10}, {11, 17}, {24, 25}, {35, 38}});
	test_merge_intervals("random 3, n=13",
		{{40, 42}, {0, 5}, {2, 6}, {23, 23}, {36, 41}, {0, 4}, {35, 35}, {39, 42}, {24, 29}, {23, 26}, {23, 24}, {32, 35}, {35, 38}},
		{{0, 6}, {23, 29}, {32, 42}});
	{
		// n = 10000, disjoint points in reverse order -> same points, ascending
		Intervals intervals;
		Intervals expected;
		for (int i{9999}; i >= 0; --i) intervals.push_back({i, i});
		for (int i{0}; i < 10000; ++i) expected.push_back({i, i});
		test_merge_intervals("n = 10000, disjoint points, reversed", intervals, expected);
	}
	{
		// n = 10000, all identical [0, 10000] -> one interval
		test_merge_intervals("n = 10000, all identical",
			Intervals(10000, {0, 10000}), {{0, 10000}});
	}
	{
		// n = 9999, touching chain [i, i+1] shuffled -> one interval [0, 9999]
		Intervals intervals;
		for (int i{0}; i < 9999; ++i) intervals.push_back({i, i + 1});
		for (size_t i{0}; i + 1 < intervals.size(); i += 2) std::swap(intervals[i], intervals[intervals.size() - 1 - i]);
		test_merge_intervals("n = 9999, touching chain", intervals, {{0, 9999}});
	}

	return 0;
}
```

Что покрывают тесты:

- **касание и соседство:** `[1,4]+[4,5]` объединяется, `[1,2]+[3,4]` нет. Это главная ловушка со строгим и нестрогим сравнением;
- **вложенность и порядок:** внутренний отрезок раньше или позже внешнего, одинаковые начала и концы, длинный отрезок в конце, который поглощает предыдущие;
- **вырожденные отрезки:** точки `[x,x]`, нулевая и максимальная границы, `n = 1`;
- **большие входы:** 10 000 несвязных точек в обратном порядке, 10 000 одинаковых отрезков, цепочка из 9 999 касающихся отрезков в перемешанном порядке.

Пришлите решение, и я разберу его с секцией `to error log`.

---

работа: 15:03 - 

---

- {1, 3}, {2, 6}, {8, 10}, {15, 18}
- [{1, 3}], {2, 6}, {3, 10}, {15, 18}
- {1, 6}, [8, 10], {15, 18}
- {1, 6}, {8, 10}, [15, 18]


```cpp
#include <algorithm>
#include <iostream>
#include <format>
#include <vector>
#include <string>
#include <ranges>

namespace {
	using Intervals = std::vector<std::vector<int>>;

	Intervals get_merged_intervals(const Intervals& intervals_) {
		Intervals intervals{intervals_};
		std::ranges::sort(intervals);

		Intervals result;
		for (auto& hop: intervals) {
			if (result.empty() || result.back()[1] < hop[0]) {
				result.push_back(std::move(hop));
			} else {
				result.back()[1] = std::max(result.back()[1], hop[1]);
			}
		}
		
		return result;
	}

	void test_merge_intervals(const std::string& label,
							  const Intervals& intervals,
							  const Intervals& expected) {
		auto result{get_merged_intervals(intervals)};
		std::ranges::sort(result);   // порядок в ответе не важен
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_merge_intervals("description example 1",
		{{1, 3}, {2, 6}, {8, 10}, {15, 18}},
		{{1, 6}, {8, 10}, {15, 18}});
	test_merge_intervals("description example 2 (touching)",
		{{1, 4}, {4, 5}},
		{{1, 5}});
	test_merge_intervals("single interval",
		{{5, 7}},
		{{5, 7}});
	test_merge_intervals("single point interval",
		{{3, 3}},
		{{3, 3}});
	test_merge_intervals("two disjoint, sorted",
		{{1, 2}, {4, 5}},
		{{1, 2}, {4, 5}});
	test_merge_intervals("two disjoint, unsorted",
		{{4, 5}, {1, 2}},
		{{1, 2}, {4, 5}});
	test_merge_intervals("adjacent integers do not merge",
		{{1, 2}, {3, 4}},
		{{1, 2}, {3, 4}});
	test_merge_intervals("identical intervals",
		{{2, 5}, {2, 5}, {2, 5}},
		{{2, 5}});
	test_merge_intervals("nested",
		{{1, 10}, {2, 3}, {4, 5}, {6, 9}},
		{{1, 10}});
	test_merge_intervals("nested, inner first",
		{{2, 3}, {1, 10}},
		{{1, 10}});
	test_merge_intervals("touching chain",
		{{1, 2}, {2, 3}, {3, 4}, {4, 5}},
		{{1, 5}});
	test_merge_intervals("all merge, unsorted",
		{{6, 9}, {1, 4}, {3, 7}, {8, 12}},
		{{1, 12}});
	test_merge_intervals("point inside interval",
		{{1, 5}, {3, 3}},
		{{1, 5}});
	test_merge_intervals("points only",
		{{7, 7}, {2, 2}, {7, 7}, {5, 5}},
		{{2, 2}, {5, 5}, {7, 7}});
	test_merge_intervals("same start, different end",
		{{1, 4}, {1, 2}, {1, 9}},
		{{1, 9}});
	test_merge_intervals("same end, different start",
		{{1, 9}, {5, 9}, {3, 9}},
		{{1, 9}});
	test_merge_intervals("zero start",
		{{0, 0}, {0, 1}, {2, 3}},
		{{0, 1}, {2, 3}});
	test_merge_intervals("max value",
		{{10000, 10000}, {9999, 10000}, {0, 1}},
		{{0, 1}, {9999, 10000}});
	test_merge_intervals("late long interval absorbs earlier",
		{{1, 2}, {3, 4}, {5, 6}, {0, 10}},
		{{0, 10}});
	test_merge_intervals("mixed groups",
		{{9, 11}, {1, 3}, {12, 14}, {2, 4}, {5, 5}, {10, 13}, {20, 22}, {21, 25}},
		{{1, 4}, {5, 5}, {9, 14}, {20, 25}});
	test_merge_intervals("random 1, n=14",
		{{35, 35}, {30, 35}, {33, 35}, {34, 37}, {14, 14}, {23, 27}, {4, 6}, {16, 20}, {13, 13}, {36, 37}, {3, 7}, {35, 36}, {24, 29}, {1, 2}},
		{{1, 2}, {3, 7}, {13, 13}, {14, 14}, {16, 20}, {23, 29}, {30, 37}});
	test_merge_intervals("random 2, n=11",
		{{15, 16}, {6, 10}, {12, 16}, {13, 16}, {11, 16}, {24, 25}, {4, 10}, {13, 17}, {11, 12}, {11, 12}, {35, 38}},
		{{4, 10}, {11, 17}, {24, 25}, {35, 38}});
	test_merge_intervals("random 3, n=13",
		{{40, 42}, {0, 5}, {2, 6}, {23, 23}, {36, 41}, {0, 4}, {35, 35}, {39, 42}, {24, 29}, {23, 26}, {23, 24}, {32, 35}, {35, 38}},
		{{0, 6}, {23, 29}, {32, 42}});
	{
		// n = 10000, disjoint points in reverse order -> same points, ascending
		Intervals intervals;
		Intervals expected;
		for (int i{9999}; i >= 0; --i) intervals.push_back({i, i});
		for (int i{0}; i < 10000; ++i) expected.push_back({i, i});
		test_merge_intervals("n = 10000, disjoint points, reversed", intervals, expected);
	}
	{
		// n = 10000, all identical [0, 10000] -> one interval
		test_merge_intervals("n = 10000, all identical",
			Intervals(10000, {0, 10000}), {{0, 10000}});
	}
	{
		// n = 9999, touching chain [i, i+1] shuffled -> one interval [0, 9999]
		Intervals intervals;
		for (int i{0}; i < 9999; ++i) intervals.push_back({i, i + 1});
		for (size_t i{0}; i + 1 < intervals.size(); i += 2) std::swap(intervals[i], intervals[intervals.size() - 1 - i]);
		test_merge_intervals("n = 9999, touching chain", intervals, {{0, 9999}});
	}

	return 0;
}
```
