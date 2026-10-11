## Задача: Valid Anagram (LeetCode 242, easy)

**Условие.** Даны две строки `s` и `t`. Нужно вернуть `true`, если `t` является анаграммой `s`, то есть получается перестановкой всех букв `s`, и `false` иначе. Каждая буква должна встречаться в `t` столько же раз, сколько в `s`.

**Сигнатура:**

```cpp
bool isAnagram(std::string s, std::string t);
```

**Ограничения:**

- `1 <= s.size(), t.size() <= 5 * 10^4`
- `s` и `t` состоят только из строчных латинских букв `a`-`z`

**Что обычно спрашивает интервьюер:**

1. Решение в лоб и его сложность. Можно ли сделать быстрее без сортировки?
2. Решение через сортировку обеих строк. Сложность по времени и по памяти? Можно ли не менять входные строки?
3. Решение через счётчики. Какой контейнер выбрать: `std::unordered_map` или массив? Почему здесь подходит массив из 26 элементов?
4. Нужно ли проверять длины заранее? Что даёт ранний выход?
5. Можно ли обойтись одним массивом счётчиков, а не двумя? Как организовать проверку: сначала прибавлять, потом вычитать?
6. Почему нельзя сравнивать суммы кодов символов или их XOR? Приведите контрпример.
7. Follow-up: что изменится, если строки содержат Unicode (в UTF-8 или как `char32_t`)? Какой контейнер тогда нужен и какая будет память?

**Целевая сложность:** O(n) по времени и O(1) дополнительной памяти (массив из 26 счётчиков); либо O(n log n) по времени при сортировке.

## Шаблон `main`

Решения в шаблоне нет, рабочий метод пустой и возвращает `false`. Из-за этого 15 тестов с ожидаемым `false` проходят случайно, а все 16 тестов с ожидаемым `true` падают. Это нормально для заглушки. Ожидаемые значения проверены независимо (сравнение отсортированных строк в Python), эталонное решение проходит все 31 тест под `-fsanitize=address,undefined`, шаблон собирается без предупреждений.

```cpp
namespace {
	bool get_is_anagram([[maybe_unused]] const std::string& s, [[maybe_unused]] const std::string& t) {
		return false;
	}

	void test_is_anagram(const std::string& label,
						 const std::string& s,
						 const std::string& t,
						 const bool expected) {
		const auto result{get_is_anagram(s, t)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_is_anagram("description example 1",
		"anagram", "nagaram",
		true);
	test_is_anagram("description example 2",
		"rat", "car",
		false);
	test_is_anagram("single char, equal",
		"a", "a",
		true);
	test_is_anagram("single char, different",
		"a", "b",
		false);
	test_is_anagram("equal strings",
		"abc", "abc",
		true);
	test_is_anagram("reversed string",
		"stressed", "desserts",
		true);
	test_is_anagram("different lengths, t shorter",
		"ab", "a",
		false);
	test_is_anagram("different lengths, t longer",
		"a", "ab",
		false);
	test_is_anagram("same letters, different counts",
		"aab", "abb",
		false);
	test_is_anagram("same set, different counts, same length",
		"aacc", "ccac",
		false);
	test_is_anagram("same sum of codes (ad vs bc)",
		"ad", "bc",
		false);
	test_is_anagram("same xor of codes (aa vs bb)",
		"aa", "bb",
		false);
	test_is_anagram("all same letter, equal",
		"zzzzz", "zzzzz",
		true);
	test_is_anagram("all same letter, one different",
		"zzzzz", "zzzzy",
		false);
	test_is_anagram("boundary letters a and z",
		"azaz", "zzaa",
		true);
	test_is_anagram("classic listen/silent",
		"listen", "silent",
		true);
	test_is_anagram("each letter twice, rearranged",
		"aabbcc", "abcabc",
		true);
	test_is_anagram("palindrome vs rearrangement",
		"abcba", "bacab",
		true);
	test_is_anagram("one letter replaced",
		"abcdefg", "abcdefh",
		false);
	test_is_anagram("pangram and its reversal",
		"thequickbrownfoxjumpsoverthelazydog", "godyzalehtrevospmujxofnworbkciuqeht",
		true);
	test_is_anagram("random permutation 1, n=15",
		"adeedebabcebbed", "bedadbdeebecaeb",
		true);
	test_is_anagram("random permutation 2, n=15",
		"cabebaaebbceeeb", "ceeabaecbbebbea",
		true);
	test_is_anagram("random permutation 3, n=14",
		"ebeedbedceeaeb", "deeaebeeebdbce",
		true);
	test_is_anagram("random, one letter replaced 1, n=10",
		"aeeeecbbad", "aeebecbbda",
		false);
	test_is_anagram("random, one letter replaced 2, n=16",
		"cacaceedbbdaaabe", "aaaedccbdceaabce",
		false);
	test_is_anagram("random, one letter replaced 3, n=15",
		"ddeabeadcccbcdc", "acebddbcaccddeb",
		false);
	{
		// n = 50000, 26 letters in a repeating pattern, t is s reversed -> anagram
		std::string s(50000, 'a');
		for (int i{0}; i < 50000; ++i) s[i] = static_cast<char>('a' + i % 26);
		const std::string t{s.rbegin(), s.rend()};
		test_is_anagram("n = 50000, reversed", s, t, true);
	}
	{
		// same s, t differs by one letter (an 'a' replaced by 'b') -> not an anagram
		std::string s(50000, 'a');
		for (int i{0}; i < 50000; ++i) s[i] = static_cast<char>('a' + i % 26);
		std::string t{s};
		t[0] = 'b';
		test_is_anagram("n = 50000, one letter replaced", s, t, false);
	}
	{
		// n = 50000, all 'z'
		test_is_anagram("n = 50000, all equal", std::string(50000, 'z'), std::string(50000, 'z'), true);
	}
	{
		// n = 50000, 49999 'a' + 'b' against 'b' + 49999 'a' -> anagram
		const std::string s{std::string(49999, 'a') + 'b'};
		const std::string t{'b' + std::string(49999, 'a')};
		test_is_anagram("n = 50000, one different letter moved", s, t, true);
	}
	{
		// n = 50000, 49999 'a' + 'b' against 50000 'a' -> not an anagram
		const std::string s{std::string(49999, 'a') + 'b'};
		test_is_anagram("n = 50000, one different letter", s, std::string(50000, 'a'), false);
	}

	return 0;
}
```

Что покрывают тесты:

- **ловушки для «хитрых» проверок:** `ad` против `bc` (одинаковая сумма кодов, но не анаграмма) и `aa` против `bb` (одинаковый XOR). Решение «сравнить суммы» или «сравнить XOR» падает;
- **разные длины и одинаковый набор букв:** `ab` против `a`, `aab` против `abb`, `aacc` против `ccac`. Решение, которое проверяет только наличие букв, а не их количество, падает;
- **границы алфавита и размера:** буквы `a` и `z`, `n = 1`, пять тестов на `n = 50 000` (разворот, замена одной буквы, все равны, одна другая буква);
- **случайные перестановки и перестановки с одной заменой** для строк на пяти буквах: они ловят ошибки счётчиков, которые не видны на коротких примерах.

Пришлите решение, и я разберу его с секцией `to error log`.

---

work - 14:12 -14:21

---


```cpp
#include <cstddef>
#include <iostream>
#include <format>
#include <string>
#include <array>

namespace {
	bool get_is_anagram(const std::string& s, const std::string& t) {
		if (s.size() != t.size()) return false;
	
		std::array<int, 26> char_counters{0};
		for (std::size_t i{0}; i < s.size(); ++i) {
			++char_counters[static_cast<int>(s[i] - 'a')];
			--char_counters[static_cast<int>(t[i] - 'a')];
		}
		
		for (const auto count: char_counters) {
			if (count != 0) return false;
		}
	
		return true;
	}

	void test_is_anagram(const std::string& label,
						 const std::string& s,
						 const std::string& t,
						 const bool expected) {
		const auto result{get_is_anagram(s, t)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_is_anagram("description example 1",
		"anagram", "nagaram",
		true);
	test_is_anagram("description example 2",
		"rat", "car",
		false);
	test_is_anagram("single char, equal",
		"a", "a",
		true);
	test_is_anagram("single char, different",
		"a", "b",
		false);
	test_is_anagram("equal strings",
		"abc", "abc",
		true);
	test_is_anagram("reversed string",
		"stressed", "desserts",
		true);
	test_is_anagram("different lengths, t shorter",
		"ab", "a",
		false);
	test_is_anagram("different lengths, t longer",
		"a", "ab",
		false);
	test_is_anagram("same letters, different counts",
		"aab", "abb",
		false);
	test_is_anagram("same set, different counts, same length",
		"aacc", "ccac",
		false);
	test_is_anagram("same sum of codes (ad vs bc)",
		"ad", "bc",
		false);
	test_is_anagram("same xor of codes (aa vs bb)",
		"aa", "bb",
		false);
	test_is_anagram("all same letter, equal",
		"zzzzz", "zzzzz",
		true);
	test_is_anagram("all same letter, one different",
		"zzzzz", "zzzzy",
		false);
	test_is_anagram("boundary letters a and z",
		"azaz", "zzaa",
		true);
	test_is_anagram("classic listen/silent",
		"listen", "silent",
		true);
	test_is_anagram("each letter twice, rearranged",
		"aabbcc", "abcabc",
		true);
	test_is_anagram("palindrome vs rearrangement",
		"abcba", "bacab",
		true);
	test_is_anagram("one letter replaced",
		"abcdefg", "abcdefh",
		false);
	test_is_anagram("pangram and its reversal",
		"thequickbrownfoxjumpsoverthelazydog", "godyzalehtrevospmujxofnworbkciuqeht",
		true);
	test_is_anagram("random permutation 1, n=15",
		"adeedebabcebbed", "bedadbdeebecaeb",
		true);
	test_is_anagram("random permutation 2, n=15",
		"cabebaaebbceeeb", "ceeabaecbbebbea",
		true);
	test_is_anagram("random permutation 3, n=14",
		"ebeedbedceeaeb", "deeaebeeebdbce",
		true);
	test_is_anagram("random, one letter replaced 1, n=10",
		"aeeeecbbad", "aeebecbbda",
		false);
	test_is_anagram("random, one letter replaced 2, n=16",
		"cacaceedbbdaaabe", "aaaedccbdceaabce",
		false);
	test_is_anagram("random, one letter replaced 3, n=15",
		"ddeabeadcccbcdc", "acebddbcaccddeb",
		false);
	{
		// n = 50000, 26 letters in a repeating pattern, t is s reversed -> anagram
		std::string s(50000, 'a');
		for (int i{0}; i < 50000; ++i) s[i] = static_cast<char>('a' + i % 26);
		const std::string t{s.rbegin(), s.rend()};
		test_is_anagram("n = 50000, reversed", s, t, true);
	}
	{
		// same s, t differs by one letter (an 'a' replaced by 'b') -> not an anagram
		std::string s(50000, 'a');
		for (int i{0}; i < 50000; ++i) s[i] = static_cast<char>('a' + i % 26);
		std::string t{s};
		t[0] = 'b';
		test_is_anagram("n = 50000, one letter replaced", s, t, false);
	}
	{
		// n = 50000, all 'z'
		test_is_anagram("n = 50000, all equal", std::string(50000, 'z'), std::string(50000, 'z'), true);
	}
	{
		// n = 50000, 49999 'a' + 'b' against 'b' + 49999 'a' -> anagram
		const std::string s{std::string(49999, 'a') + 'b'};
		const std::string t{'b' + std::string(49999, 'a')};
		test_is_anagram("n = 50000, one different letter moved", s, t, true);
	}
	{
		// n = 50000, 49999 'a' + 'b' against 50000 'a' -> not an anagram
		const std::string s{std::string(49999, 'a') + 'b'};
		test_is_anagram("n = 50000, one different letter", s, std::string(50000, 'a'), false);
	}

	return 0;
}
```