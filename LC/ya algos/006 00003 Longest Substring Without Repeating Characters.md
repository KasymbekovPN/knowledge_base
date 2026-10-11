
# 3. Longest Substring Without Repeating Characters (Medium)

Дана строка `s`. Нужно найти **длину самой длинной подстроки без повторяющихся символов**.

**Подстрока** это непрерывная последовательность символов строки. Пропускать символы нельзя, поэтому, например, `"pwke"` в `"pwwkew"` подстрокой не является: это подпоследовательность.

**Ограничения:** `0 ≤ s.length ≤ 5·10⁴`, строка состоит из английских букв, цифр, символов и пробелов.

**Примеры:**

```
Вход:  s = "abcabcbb"
Выход: 3
Пояснение: ответ "abc", длина 3.
```

```
Вход:  s = "bbbbb"
Выход: 1
Пояснение: ответ "b", длина 1.
```

```
Вход:  s = "pwwkew"
Выход: 3
Пояснение: ответ "wke", длина 3. Строка "pwke" это подпоследовательность, а не подстрока.
```

Ориентир по времени: 30 минут. Начните с примеров и крайних случаев: пустая строка, один символ, все символы одинаковые, все символы различные, повтор в самом конце, повтор через несколько символов (`"abba"`, `"tmmzuxt"`). Назовите подход и сложность вслух: сначала простой вариант и его цену, затем оптимизацию. Затем код в блокноте с тестами. Пришлите код, разберу.

---

работа 16:50 - 17:18

---

# пустая строка

- ret

# один символ

- main

# все символы одинаковые
 - main

# все символы различные
- main

# повтор в самом конце

- [a]bcddcabcd => (a) (a)
- a[b]cddcabcd => (ab) (ab)
- ab[c]ddcabcd => (abc) (abc)
- abc[d]dcabcd => (abcd) (abcd)
- abcd[d]cabcd => (d) (abcd)
- abcdd[c]abcd => (dc) (abcd)
- abcddc[a]bcd => (dca) (abcd)
- abcddca[b]cd => (dcab) (abcd)
- abcddcab[c]d => (c) (abcd)
- abcddcabc[d] => (cd) (abcd)

# повтор через несколько символов (`"abba"`, `"tmmzuxt"`)

# abcabcbb (main)

 current, max_len

- [a]bcabcbb => (a) (a)
- a[b]cabcbb => (ab) (ab)
- ab[c]abcbb => (abc) (abc)
- abc[a]bcbb => (a) (abc)
- abca[b]cbb => (ab) (abc)
- abcab[c]bb => (abc) (abc)
- abcabc[b]b => (b) (abc)
- abcabcb[b] => (b) (abc)

---

left (), right []

- (a)[b]cabcbb => 0, 1
- (a)b[c]abcbb => 0, 2
- a(b)c[a]bcbb => 1, 3
- ab(c)a[b]cbb => 2, 4
- abc(a)b[c]bb => 3, 5
- abcab(c)[b]b => 3, 5

---


```cpp

#include <cstddef>
#include <string>
#include <vector>
#include <iostream>
#include <format>
#include <algorithm>
#include <array>

namespace {
	// вектор не большой - будет работать быстрее set
	void shrink_past_previous(const char ch, std::vector<char>& container) {
		auto it{std::find(container.begin(), container.end(), ch)};
		if (it == container.end()) return;
	
		container.erase(container.begin(), it + 1);
	}

	int get_longest_substring(const std::string& s) {
		const std::size_t size{s.size()};
	
		std::vector<char> symbols;
		symbols.reserve(s.size());

		std::size_t total_best{0};
		for (std::size_t i{0}; i < size; ++i) {
			const auto ch{s[i]};
			shrink_past_previous(ch, symbols);
			
			symbols.push_back(ch);
			total_best = std::max(total_best, symbols.size());
		}
		
		return static_cast<int>(total_best);
	}
	
	int get_longest_substring_window(const std::string& s) {
		std::aray<bool, 256> in_win{};
		std::size_t left{0};
		std::size_t best{0};
		
		for (std::size_t right{0}; right < s.size(); ++right) {
			const auto ch{static_cast<unsigned char>(s[right])};
			while (in_win[ch]) {
				in_win[static_cast<unsigned char>(s[left])] = false;
				++left;
			}
			in_win[ch] = true;
			best = std::max(best, right - left + 1);
		}
		
		return static_cast<int>(best);
	}
}
	
	
	void test_get_longest_substring(const std::string& label,
									const std::string& s,
									const int expected) {
		const auto result{get_longest_substring(s)};
		std::cout <<std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_get_longest_substring("repeat 0", "abcabcbb", 3);
	test_get_longest_substring("alone symbol", "bbbbb", 1);
	test_get_longest_substring("once sub", "pwwkew", 3);
	test_get_longest_substring("empty", "", 0);
	test_get_longest_substring("one symbol", "x", 1);
	test_get_longest_substring("every unique", "abcde", 5);
	test_get_longest_substring("repeat 1", "abcddcabcd", 4);

	return 0;
}

```