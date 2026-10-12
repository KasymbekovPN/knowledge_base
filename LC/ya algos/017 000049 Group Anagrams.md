## 49. Group Anagrams (Medium)

Дан массив строк `strs`. Нужно сгруппировать **анаграммы** вместе. Ответ можно вернуть в любом порядке: и порядок групп, и порядок строк внутри группы произвольный.

Анаграмма — слово, составленное из тех же букв в том же количестве, но, возможно, в другом порядке.

**Примеры**

- `strs = ["eat","tea","tan","ate","nat","bat"]` → `[["bat"],["nat","tan"],["ate","eat","tea"]]`
- `strs = [""]` → `[[""]]`
- `strs = ["a"]` → `[["a"]]`

**Сигнатура**

```cpp
std::vector<std::vector<std::string>> group_anagrams(const std::vector<std::string>& strs);
```

**Ограничения**

- `1 <= strs.length <= 10^4`
- `0 <= strs[i].length <= 100`
- `strs[i]` состоит из строчных английских букв.
- В массиве могут быть повторы и пустые строки.

**Вопросы интервьюера**

1. Какой ключ вы выберете для группы и почему он однозначно определяет класс анаграмм?
2. Сколько стоит построить ключ: сортировкой и подсчётом букв? Какой из вариантов лучше при длинных строках и при длинном алфавите?
3. Можно ли использовать сумму кодов символов или произведение простых чисел как ключ? Что может пойти не так?
4. Если вы строите ключ из счётчиков, как не перепутать счётчики `1,11` и `11,1`?
5. Какой тип выбрать для ключа в `unordered_map`, если ключ — массив из 26 счётчиков? Как вычислить для него хеш?
6. Что изменится, если строки очень длинные, а различных групп мало?

**Целевая сложность:** O(n · k log k) при ключе-сортировке или O(n · k) при ключе-счётчиках, где k — максимальная длина строки. Память O(n · k).

**Шаблон**

```cpp
namespace {
	std::vector<std::vector<std::string>> group_anagrams(
		[[maybe_unused]] const std::vector<std::string>& strs) {
		return {};
	}

	void normalize(std::vector<std::vector<std::string>>& groups) {
		for (auto& group : groups) std::ranges::sort(group);
		std::ranges::sort(groups);
	}

	void test_group_anagrams(const std::string& label,
							 const std::vector<std::string>& strs,
							 std::vector<std::vector<std::string>> expected) {
		auto result{group_anagrams(strs)};
		normalize(result);
		normalize(expected);
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_group_anagrams("description example 1",
		{"eat", "tea", "tan", "ate", "nat", "bat"},
		{{"eat", "tea", "ate"}, {"tan", "nat"}, {"bat"}});
	test_group_anagrams("description example 2",
		{""},
		{{""}});
	test_group_anagrams("description example 3",
		{"a"},
		{{"a"}});
	test_group_anagrams("two empty strings",
		{"", ""},
		{{"", ""}});
	test_group_anagrams("empty strings mixed with a letter",
		{"", "a", ""},
		{{"", ""}, {"a"}});
	test_group_anagrams("no anagrams",
		{"abc", "def", "ghi"},
		{{"abc"}, {"def"}, {"ghi"}});
	test_group_anagrams("all permutations of abc",
		{"abc", "bca", "cab", "acb", "bac", "cba"},
		{{"abc", "bca", "cab", "acb", "bac", "cba"}});
	test_group_anagrams("duplicates",
		{"abc", "abc", "bca"},
		{{"abc", "abc", "bca"}});
	test_group_anagrams("same sum of letters, different multisets",
		{"ad", "bc"},
		{{"ad"}, {"bc"}});
	test_group_anagrams("same sum of letters, one letter doubled",
		{"ac", "bb"},
		{{"ac"}, {"bb"}});
	test_group_anagrams("counts 1,11 vs 11,1 must not merge",
		{"abbbbbbbbbbb", "aaaaaaaaaaab"},
		{{"abbbbbbbbbbb"}, {"aaaaaaaaaaab"}});
	test_group_anagrams("same letters, different lengths",
		{"aab", "ab", "aabb", "ba"},
		{{"aab"}, {"ab", "ba"}, {"aabb"}});
	test_group_anagrams("prefixes are not anagrams",
		{"abc", "abcd", "abcde"},
		{{"abc"}, {"abcd"}, {"abcde"}});
	test_group_anagrams("same letters, different counts",
		{"aabc", "abbc", "abcc"},
		{{"aabc"}, {"abbc"}, {"abcc"}});
	test_group_anagrams("mixed words",
		{"listen", "silent", "enlist", "google", "gogole", "inlets", "banana", "nabana"},
		{{"listen", "silent", "enlist", "inlets"}, {"google", "gogole"}, {"banana", "nabana"}});
	test_group_anagrams("interleaved groups",
		{"ab", "cd", "ba", "dc", "ab"},
		{{"ab", "ba", "ab"}, {"cd", "dc"}});
	test_group_anagrams("whole alphabet, its reversal, and a near miss",
		{"abcdefghijklmnopqrstuvwxyz", "zyxwvutsrqponmlkjihgfedcba", "abcdefghijklmnopqrstuvwxyy"},
		{{"abcdefghijklmnopqrstuvwxyz", "zyxwvutsrqponmlkjihgfedcba"}, {"abcdefghijklmnopqrstuvwxyy"}});
	test_group_anagrams("single letter repeated different times",
		{"a", "aa", "aaa", "aa", "a"},
		{{"a", "a"}, {"aa", "aa"}, {"aaa"}});
	test_group_anagrams("random 1, n=15",
		{"aabcad", "ee", "ad", "da", "aaadcb", "aacbad", "aabcda", "da", "da", "ee", "caadab", "cbdaaa", "ad", "bdaaca", "da"},
		{{"aabcad", "aaadcb", "aacbad", "aabcda", "caadab", "cbdaaa", "bdaaca"}, {"ee", "ee"}, {"ad", "da", "da", "da", "ad", "da"}});
	test_group_anagrams("random 2, n=10",
		{"ddaed", "dadde", "aa", "dbdcdc", "eddda", "ddade", "dedda", "deadd", "aa", "aa"},
		{{"ddaed", "dadde", "eddda", "ddade", "dedda", "deadd"}, {"aa", "aa", "aa"}, {"dbdcdc"}});
	test_group_anagrams("random 3, n=13",
		{"cea", "a", "da", "a", "da", "ad", "a", "da", "a", "a", "cae", "a", "a"},
		{{"cea", "cae"}, {"a", "a", "a", "a", "a", "a", "a"}, {"da", "da", "ad", "da"}});

	{
		// n = 10000, all strings are empty -> one group
		const std::vector<std::string> strs(10000, "");
		test_group_anagrams("n = 10000, all empty",
			strs,
			{strs});
	}
	{
		// n = 10000, 4 letters with counts summing to 100, all multisets different -> 10000 singletons
		std::vector<std::string> strs;
		std::vector<std::vector<std::string>> expected;
		for (int a{0}; a <= 100 && strs.size() < 10000; ++a) {
			for (int b{0}; a + b <= 100 && strs.size() < 10000; ++b) {
				for (int c{0}; a + b + c <= 100 && strs.size() < 10000; ++c) {
					std::string s;
					s += std::string(static_cast<std::size_t>(a), 'a');
					s += std::string(static_cast<std::size_t>(b), 'b');
					s += std::string(static_cast<std::size_t>(c), 'c');
					s += std::string(static_cast<std::size_t>(100 - a - b - c), 'd');
					std::rotate(s.begin(), s.begin() + (a + 2 * b + 3 * c) % 100, s.end());
					expected.push_back({s});
					strs.push_back(s);
				}
			}
		}
		test_group_anagrams("n = 10000, length 100, all different multisets",
			strs,
			expected);
	}
	{
		// n = 10000, 100 groups of 100 rotations of 'a'*g + 'b'*(100-g), g = 1..100
		std::vector<std::string> strs;
		std::vector<std::vector<std::string>> expected(100);
		for (int k{0}; k < 100; ++k) {
			for (int g{1}; g <= 100; ++g) {
				std::string s(static_cast<std::size_t>(g), 'a');
				s += std::string(static_cast<std::size_t>(100 - g), 'b');
				std::rotate(s.begin(), s.begin() + k, s.end());
				strs.push_back(s);
				expected[static_cast<std::size_t>(g - 1)].push_back(s);
			}
		}
		test_group_anagrams("n = 10000, 100 groups of 100 rotations", strs, expected);
	}
	{
		// n = 10000, all rotations of one 100-char string -> one group
		std::string base;
		for (int i{0}; i < 100; ++i) base += static_cast<char>('a' + (i * i) % 26);
		std::vector<std::string> strs;
		for (int i{0}; i < 10000; ++i) {
			std::string s{base};
			std::rotate(s.begin(), s.begin() + i % 100, s.end());
			strs.push_back(s);
		}
		test_group_anagrams("n = 10000, one big group of rotations",
			strs,
			{strs});
	}

	return 0;
}
```

Порядок групп и строк внутри групп тест не учитывает: он сортирует и ваш ответ, и ожидаемый. Пришлите свою попытку, и я её разберу.

---

work: 12:13 - 12:47

---

- ["eat"], "tea", "tan", "ate", "nat", "bat" => {a,e,t : {eat}}
- "eat", ["tea"], "tan", "ate", "nat", "bat" => {a,e,t : {eat, tea}}
- "eat", "tea", ["tan"], "ate", "nat", "bat" => {a,e,t : {eat, tea}, a,n,t: {tan}}

---

```cpp
#include <iostream>
#include <format>
#include <algorithm>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstddef>

namespace {
	constexpr std::size_t ALPHABET_SIZE{256};

	struct CountKeyHash{
		std::size_t operator()(const std::array<int, ALPHABET_SIZE>& key) const {
			std::size_t h{0};
			for (const auto c: key) {
				h = h * 31 + static_cast<std::size_t>(c);
			}
			
			return h;
		}
	};

	std::vector<std::vector<std::string>> group_anagrams_str_key(
		const std::vector<std::string>& strs) {

		const auto size{strs.size()};

		std::vector<std::vector<std::string>> result;
		result.reserve(size);

		std::unordered_map<std::string, std::size_t> keys_by_idxs;
		keys_by_idxs.reserve(size);

		for (const auto& str: strs) {
			auto key{str};
			std::ranges::sort(key);
			
			const auto [it, inserted]{idx_by_key.try_emplace(key, result.size())};
			if (inserted) {
				result.push_back({str});
			} else {
				result[it->second].push_back(str);
			}
		}

		return result;
	}
	
	std::vector<std::vector<std::string>> group_anagrams_count_key(
		const std::vector<std::string>& strs) {

		const auto size{strs.size()};

		std::vector<std::vector<std::string>> result;
		result.reserve(size);

		std::unordered_map<
			std::array<int, ALPHABET_SIZE>,
			std::size_t,
			CountKeyHash> keys_by_idxs;
		keys_by_idxs.reserve(size);

		for (const auto& str: strs) {
			std::array<int, ALPHABET_SIZE> key{};
			for (const auto ch: str) {
				const auto idx{
					static_cast<std::size_t>(
						static_cast<unsigned char>(ch)
					)
				};
				++key[idx];
			}
			
			const auto [it, inserted]{keys_by_idxs.try_emplace(key, result.size())};
			if (inserted) {
				result.push_back({str});
			} else {
				result[it->second].push_back(str);
			}
		}

		return result;
	}

	void normalize(std::vector<std::vector<std::string>>& groups) {
		for (auto& group : groups) std::ranges::sort(group);
		std::ranges::sort(groups);
	}

	void test_group_anagrams(const std::string& label,
							 const std::vector<std::string>& strs,
							 std::vector<std::vector<std::string>> expected) {
		normalize(expected);

		auto result_str_key{group_anagrams_str_key(strs)};
		normalize(result_str_key);
		
		auto result_count_key{group_anagrams_count_key(strs)};
		normalize(result_count_key);
		
		std::cout << std::format("{}: str_key: {}, count_key: {}\n",
			label,
			result_str_key == expected ? "OK" : "FAIL",
			result_count_key == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_group_anagrams("description example 1",
		{"eat", "tea", "tan", "ate", "nat", "bat"},
		{{"eat", "tea", "ate"}, {"tan", "nat"}, {"bat"}});
	test_group_anagrams("description example 2",
		{""},
		{{""}});
	test_group_anagrams("description example 3",
		{"a"},
		{{"a"}});
	test_group_anagrams("two empty strings",
		{"", ""},
		{{"", ""}});
	test_group_anagrams("empty strings mixed with a letter",
		{"", "a", ""},
		{{"", ""}, {"a"}});
	test_group_anagrams("no anagrams",
		{"abc", "def", "ghi"},
		{{"abc"}, {"def"}, {"ghi"}});
	test_group_anagrams("all permutations of abc",
		{"abc", "bca", "cab", "acb", "bac", "cba"},
		{{"abc", "bca", "cab", "acb", "bac", "cba"}});
	test_group_anagrams("duplicates",
		{"abc", "abc", "bca"},
		{{"abc", "abc", "bca"}});
	test_group_anagrams("same sum of letters, different multisets",
		{"ad", "bc"},
		{{"ad"}, {"bc"}});
	test_group_anagrams("same sum of letters, one letter doubled",
		{"ac", "bb"},
		{{"ac"}, {"bb"}});
	test_group_anagrams("counts 1,11 vs 11,1 must not merge",
		{"abbbbbbbbbbb", "aaaaaaaaaaab"},
		{{"abbbbbbbbbbb"}, {"aaaaaaaaaaab"}});
	test_group_anagrams("same letters, different lengths",
		{"aab", "ab", "aabb", "ba"},
		{{"aab"}, {"ab", "ba"}, {"aabb"}});
	test_group_anagrams("prefixes are not anagrams",
		{"abc", "abcd", "abcde"},
		{{"abc"}, {"abcd"}, {"abcde"}});
	test_group_anagrams("same letters, different counts",
		{"aabc", "abbc", "abcc"},
		{{"aabc"}, {"abbc"}, {"abcc"}});
	test_group_anagrams("mixed words",
		{"listen", "silent", "enlist", "google", "gogole", "inlets", "banana", "nabana"},
		{{"listen", "silent", "enlist", "inlets"}, {"google", "gogole"}, {"banana", "nabana"}});
	test_group_anagrams("interleaved groups",
		{"ab", "cd", "ba", "dc", "ab"},
		{{"ab", "ba", "ab"}, {"cd", "dc"}});
	test_group_anagrams("whole alphabet, its reversal, and a near miss",
		{"abcdefghijklmnopqrstuvwxyz", "zyxwvutsrqponmlkjihgfedcba", "abcdefghijklmnopqrstuvwxyy"},
		{{"abcdefghijklmnopqrstuvwxyz", "zyxwvutsrqponmlkjihgfedcba"}, {"abcdefghijklmnopqrstuvwxyy"}});
	test_group_anagrams("single letter repeated different times",
		{"a", "aa", "aaa", "aa", "a"},
		{{"a", "a"}, {"aa", "aa"}, {"aaa"}});
	test_group_anagrams("random 1, n=15",
		{"aabcad", "ee", "ad", "da", "aaadcb", "aacbad", "aabcda", "da", "da", "ee", "caadab", "cbdaaa", "ad", "bdaaca", "da"},
		{{"aabcad", "aaadcb", "aacbad", "aabcda", "caadab", "cbdaaa", "bdaaca"}, {"ee", "ee"}, {"ad", "da", "da", "da", "ad", "da"}});
	test_group_anagrams("random 2, n=10",
		{"ddaed", "dadde", "aa", "dbdcdc", "eddda", "ddade", "dedda", "deadd", "aa", "aa"},
		{{"ddaed", "dadde", "eddda", "ddade", "dedda", "deadd"}, {"aa", "aa", "aa"}, {"dbdcdc"}});
	test_group_anagrams("random 3, n=13",
		{"cea", "a", "da", "a", "da", "ad", "a", "da", "a", "a", "cae", "a", "a"},
		{{"cea", "cae"}, {"a", "a", "a", "a", "a", "a", "a"}, {"da", "da", "ad", "da"}});

	{
		// n = 10000, all strings are empty -> one group
		const std::vector<std::string> strs(10000, "");
		test_group_anagrams("n = 10000, all empty",
			strs,
			{strs});
	}
	{
		// n = 10000, 4 letters with counts summing to 100, all multisets different -> 10000 singletons
		std::vector<std::string> strs;
		std::vector<std::vector<std::string>> expected;
		for (int a{0}; a <= 100 && strs.size() < 10000; ++a) {
			for (int b{0}; a + b <= 100 && strs.size() < 10000; ++b) {
				for (int c{0}; a + b + c <= 100 && strs.size() < 10000; ++c) {
					std::string s;
					s += std::string(static_cast<std::size_t>(a), 'a');
					s += std::string(static_cast<std::size_t>(b), 'b');
					s += std::string(static_cast<std::size_t>(c), 'c');
					s += std::string(static_cast<std::size_t>(100 - a - b - c), 'd');
					std::rotate(s.begin(), s.begin() + (a + 2 * b + 3 * c) % 100, s.end());
					expected.push_back({s});
					strs.push_back(s);
				}
			}
		}
		test_group_anagrams("n = 10000, length 100, all different multisets",
			strs,
			expected);
	}
	{
		// n = 10000, 100 groups of 100 rotations of 'a'*g + 'b'*(100-g), g = 1..100
		std::vector<std::string> strs;
		std::vector<std::vector<std::string>> expected(100);
		for (int k{0}; k < 100; ++k) {
			for (int g{1}; g <= 100; ++g) {
				std::string s(static_cast<std::size_t>(g), 'a');
				s += std::string(static_cast<std::size_t>(100 - g), 'b');
				std::rotate(s.begin(), s.begin() + k, s.end());
				strs.push_back(s);
				expected[static_cast<std::size_t>(g - 1)].push_back(s);
			}
		}
		test_group_anagrams("n = 10000, 100 groups of 100 rotations", strs, expected);
	}
	{
		// n = 10000, all rotations of one 100-char string -> one group
		std::string base;
		for (int i{0}; i < 100; ++i) base += static_cast<char>('a' + (i * i) % 26);
		std::vector<std::string> strs;
		for (int i{0}; i < 10000; ++i) {
			std::string s{base};
			std::rotate(s.begin(), s.begin() + i % 100, s.end());
			strs.push_back(s);
		}
		test_group_anagrams("n = 10000, one big group of rotations",
			strs,
			{strs});
	}

	return 0;
}
```