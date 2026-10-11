## Задача: First Unique Character in a String (LeetCode 387, easy)

**Условие.** Дана строка `s`. Нужно найти первый символ, который встречается в ней ровно один раз, и вернуть его индекс. Если такого символа нет, вернуть `-1`. «Первый» означает с наименьшим индексом в строке, а не наименьший по алфавиту.

**Сигнатура:**

```cpp
int firstUniqChar(std::string s);
```

**Ограничения:**

- `1 <= s.size() <= 10^5`
- `s` состоит только из строчных латинских букв `a`-`z`

**Что обычно спрашивает интервьюер:**

1. Решение в лоб и его сложность.
2. Решение за два прохода: сначала подсчёт, потом поиск. Какая память? Почему здесь подходит массив из 26 элементов, а не `unordered_map`?
3. Можно ли за один проход? Какие структуры для этого нужны и чем это лучше или хуже двух проходов на длинных строках?
4. Что вернуть, если уникальных символов нет? Что, если их несколько?
5. Что изменится, если алфавит не ограничен буквами (произвольные байты, Unicode)?
6. Follow-up: строка приходит потоком, и нужно уметь отвечать на вопрос «какой сейчас первый уникальный символ» после каждого символа. Какую структуру выбрать?

**Целевая сложность:** O(n) по времени и O(1) по памяти (массив из 26 счётчиков).

## Шаблон `main`

Решения в шаблоне нет, рабочий метод пустой и возвращает `-1`. Поэтому 10 тестов с ожидаемым `-1` проходят случайно, а остальные 16 падают. Это нормально для заглушки. Ожидаемые значения посчитаны в Python независимо; эталонное решение проходит все 32 теста под `-fsanitize=address,undefined`, шаблон собирается без предупреждений.

```cpp
#include <cstddef>
#include <iostream>
#include <format>
#include <string>
#include <array>
#include <algorithm>
#include <climits>

namespace {
	constexpr int NOT_SEEN{-1};
	constexpr int NOT_UNIQUE{-2};
	constexpr std::size_t ALPHABET_SIZE{256};

	int get_first_unique_char(const std::string& s) {
		std::array<int, ALPHABET_SIZE> positions;
		positions.fill(NOT_SEEN);
		
		for (std::size_t i{0}; i < s.size(); ++i) {
			const auto positions_idx{
				static_cast<std::size_t>(
					static_cast<unsigned char>(s[i]))};
			
			auto& position{positions[positions_idx]};
			if (position == NOT_UNIQUE) continue;
			else if (position == NOT_SEEN) position = static_cast<int>(i);
			else position = NOT_UNIQUE;
		}
		
		int best{INT_MAX};
		for (const auto position: positions) {
			if (position == NOT_SEEN || position == NOT_UNIQUE) continue;
			
			best = std::min(best, position);
		}
	
		return best == INT_MAX ? -1 : best;
	}

	void test_first_unique_char(const std::string& label,
								const std::string& s,
								const int expected) {
		const auto result{get_first_unique_char(s)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_first_unique_char("description example 1",
		"leetcode",
		0);
	test_first_unique_char("description example 2",
		"loveleetcode",
		2);
	test_first_unique_char("description example 3",
		"aabb",
		-1);
	test_first_unique_char("single char",
		"z",
		0);
	test_first_unique_char("two equal chars",
		"aa",
		-1);
	test_first_unique_char("two different chars",
		"ab",
		0);
	test_first_unique_char("unique at the start",
		"abb",
		0);
	test_first_unique_char("unique at the end",
		"aabbc",
		4);
	test_first_unique_char("unique in the middle",
		"aabcbdd",
		3);
	test_first_unique_char("first char repeats later",
		"abca",
		1);
	test_first_unique_char("answer is not the smallest letter",
		"cab",
		0);
	test_first_unique_char("answer is not the largest letter",
		"abc",
		0);
	test_first_unique_char("repeats are not adjacent",
		"abcabcd",
		6);
	test_first_unique_char("every char appears twice",
		"abcabc",
		-1);
	test_first_unique_char("every char appears three times",
		"abcabcabc",
		-1);
	test_first_unique_char("all same letter",
		"qqqqqqq",
		-1);
	test_first_unique_char("boundary letters a and z",
		"azazb",
		4);
	test_first_unique_char("several unique, first one wins",
		"aabbxcdyy",
		4);
	test_first_unique_char("unique char also at the very end",
		"dccbaaeb",
		0);
	test_first_unique_char("whole alphabet once",
		"abcdefghijklmnopqrstuvwxyz",
		0);
	test_first_unique_char("whole alphabet twice, no unique",
		"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
		-1);
	test_first_unique_char("random 1, n=17",
		"babbbcceaefbaabed",
		10);
	test_first_unique_char("random 2, n=23",
		"cabfeebdeadbabeeaaebebb",
		0);
	test_first_unique_char("random 3, n=10",
		"abaeeccfce",
		1);
	test_first_unique_char("random without unique 1, n=39",
		"gfaaabfcdbigbadeggfhgaaggjbeheajiffjcgj",
		-1);
	test_first_unique_char("random without unique 2, n=35",
		"eecaihccjjhcfadhibdbdgahgahfgcciedj",
		-1);
	{
		// n = 100000, all 'a' -> no unique char
		test_first_unique_char("n = 100000, all equal", std::string(100000, 'a'), -1);
	}
	{
		// n = 100000, 99999 'a' + 'b' -> unique at the very end
		test_first_unique_char("n = 100000, unique at the end",
			std::string(99999, 'a') + 'b', 99999);
	}
	{
		// n = 100000, 'b' + 99999 'a' -> unique at the very start
		test_first_unique_char("n = 100000, unique at the start",
			'b' + std::string(99999, 'a'), 0);
	}
	{
		// n = 100000, 'a' * 50000 + 'b' + 'a' * 49999 -> unique in the middle
		test_first_unique_char("n = 100000, unique in the middle",
			std::string(50000, 'a') + 'b' + std::string(49999, 'a'), 50000);
	}
	{
		// n = 100000, 26 letters in a repeating pattern, each appears thousands of times -> no unique
		std::string s(100000, 'a');
		for (int i{0}; i < 100000; ++i) s[i] = static_cast<char>('a' + i % 26);
		test_first_unique_char("n = 100000, 26-letter pattern", s, -1);
	}
	{
		// n = 99999, "ab" repeated 49999 times + 'c' -> unique at the very end
		std::string s;
		for (int i{0}; i < 49999; ++i) s += "ab";
		s += 'c';
		test_first_unique_char("n = 99999, two letters + unique tail", s, 99998);
	}

	return 0;
}
```

Что покрывают тесты:

- **«первый» значит по индексу:** `cab` и `aabbxcdyy`. Решение, которое возвращает индекс наименьшей по алфавиту уникальной буквы, падает;
- **повторы не рядом:** `abca`, `abcabcd`. Решение, которое сравнивает только соседние символы, падает;
- **нет уникальных:** строки, где каждая буква встречается два, три и больше раз, весь алфавит дважды, длинный шаблон из 26 букв. Ответ `-1`;
- **границы:** `n = 1`, буквы `a` и `z` в одной строке, пять тестов на `n = 100 000` (уникальная буква в начале, в середине, в самом конце). Квадратичное решение на последнем из них заметно зависает.

Пришлите решение, и я разберу его с секцией `to error log`.

---
