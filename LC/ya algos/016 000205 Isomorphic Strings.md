
## 205. Isomorphic Strings (Easy)

Даны две строки `s` и `t`. Нужно определить, **изоморфны** ли они.

Строки изоморфны, если символы `s` можно заменить символами `t` так, чтобы получилась `t`. При этом:

- все вхождения одного символа заменяются одним и тем же символом;
- порядок символов сохраняется;
- два разных символа не могут отображаться в один и тот же символ;
- символ может отображаться сам в себя.

**Примеры**

- `s = "egg"`, `t = "add"` → `true`
- `s = "foo"`, `t = "bar"` → `false` (`o` не может перейти и в `a`, и в `r`)
- `s = "paper"`, `t = "title"` → `true`

**Сигнатура**

```cpp
bool is_isomorphic(const std::string& s, const std::string& t);
```

**Ограничения**

- `1 <= s.length <= 5 * 10^4`
- `t.length == s.length`
- `s` и `t` состоят из допустимых ASCII-символов (буквы, цифры, пробел, знаки препинания).

**Вопросы интервьюера**

1. Почему одного отображения `s → t` недостаточно? Приведите контрпример.
2. Сколько памяти нужно, если алфавит фиксирован (ASCII), и что изменится для Unicode?
3. Можно ли решить за один проход? Каков критерий остановки?
4. Как проверить, что отображение инъективно, не заводя второй массив или таблицу?
5. Чем ваш подход отличается от сравнения «нормализованных» форм двух строк (каждый символ заменён номером первого вхождения)?

**Целевая сложность:** O(n) по времени, O(1) по памяти (размер алфавита константен).

**Шаблон**

```cpp
namespace {
	bool is_isomorphic([[maybe_unused]] const std::string& s,
					   [[maybe_unused]] const std::string& t) {
		return false;
	}

	void test_is_isomorphic(const std::string& label,
							const std::string& s,
							const std::string& t,
							const bool expected) {
		const auto result{is_isomorphic(s, t)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_is_isomorphic("description example 1",
		"egg",
		"add",
		true);
	test_is_isomorphic("description example 2",
		"foo",
		"bar",
		false);
	test_is_isomorphic("description example 3",
		"paper",
		"title",
		true);
	test_is_isomorphic("single char, same",
		"a",
		"a",
		true);
	test_is_isomorphic("single char, different",
		"a",
		"b",
		true);
	test_is_isomorphic("two chars, swap",
		"ab",
		"ba",
		true);
	test_is_isomorphic("two chars, same to different",
		"aa",
		"ab",
		false);
	test_is_isomorphic("two chars, different to same",
		"ab",
		"aa",
		false);
	test_is_isomorphic("identical strings",
		"abcdef",
		"abcdef",
		true);
	test_is_isomorphic("two different map to one",
		"badc",
		"baba",
		false);
	test_is_isomorphic("one maps to two different",
		"baba",
		"badc",
		false);
	test_is_isomorphic("pattern repeats, isomorphic",
		"abab",
		"cdcd",
		true);
	test_is_isomorphic("pattern repeats, broken at the end",
		"abab",
		"cdce",
		false);
	test_is_isomorphic("pattern repeats, broken at the start",
		"abab",
		"ccdc",
		false);
	test_is_isomorphic("all same to all same",
		"aaaa",
		"zzzz",
		true);
	test_is_isomorphic("all same to one different at the end",
		"aaaa",
		"zzzy",
		false);
	test_is_isomorphic("mapping is a cycle",
		"abc",
		"bca",
		true);
	test_is_isomorphic("cycle with a repeated first char",
		"abca",
		"bcab",
		true);
	test_is_isomorphic("conflict only on the last char",
		"abcdefga",
		"bcdefgha",
		false);
	test_is_isomorphic("conflict only on the last char (reverse side)",
		"abcdefgh",
		"bcdefghb",
		false);
	test_is_isomorphic("digits and symbols",
		"1#1!",
		"a-a+",
		true);
	test_is_isomorphic("space and punctuation",
		"a b,a",
		"x y;x",
		true);
	test_is_isomorphic("space and punctuation, broken",
		"a b,a",
		"x y;z",
		false);
	test_is_isomorphic("uppercase and lowercase are different",
		"aA",
		"bb",
		false);
	test_is_isomorphic("uppercase and lowercase, isomorphic",
		"aA",
		"bc",
		true);
	test_is_isomorphic("whole alphabet to its reversal",
		"abcdefghijklmnopqrstuvwxyz",
		"zyxwvutsrqponmlkjihgfedcba",
		true);
	test_is_isomorphic("whole alphabet to itself shifted by one",
		"abcdefghijklmnopqrstuvwxyz",
		"bcdefghijklmnopqrstuvwxyza",
		true);
	test_is_isomorphic("whole alphabet doubled, 2 to 1 at the end",
		"abcdefghijklmnopqrstuvwxyza",
		"zyxwvutsrqponmlkjihgfedcbaz",
		true);
	test_is_isomorphic("random isomorphic 1, n=21",
		"hbhhdcbddcdfaefefbbbe",
		"wjwwvajvvavehrerejjjr",
		true);
	test_is_isomorphic("random isomorphic 2, n=8",
		"bfdebdhf",
		"qofpqfuo",
		true);
	test_is_isomorphic("random isomorphic 3, n=13",
		"bcaedfeffgagh",
		"cyqgfxgxxtqtp",
		true);
	test_is_isomorphic("random, two sources to one target 1, n=9",
		"dgcbdgfee",
		"egtsegtoo",
		false);
	test_is_isomorphic("random, two sources to one target 2, n=22",
		"fcheaaehagafegfedfbdce",
		"ygezppzepepyzeyzuyvugz",
		false);
	test_is_isomorphic("random, one source to two targets 1, n=13",
		"bcfeddbbadbac",
		"sobpaasswacwo",
		false);
	test_is_isomorphic("random, one source to two targets 2, n=10",
		"gagahgfhfa",
		"lplpdbadap",
		false);

	{
		// n = 50000, all 'a' -> all 'b'
		test_is_isomorphic("n = 50000, all equal",
			std::string(50000, 'a'),
			std::string(50000, 'b'),
			true);
	}
	{
		// n = 50000, all 'a' vs all 'b' except the last char
		std::string t(50000, 'b');
		t.back() = 'c';
		test_is_isomorphic("n = 50000, all equal vs broken tail",
			std::string(50000, 'a'),
			t,
			false);
	}
	{
		// n = 50000, 94 printable chars in a cycle -> shifted by one
		std::string s(50000, ' ');
		std::string t(50000, ' ');
		for (int i{0}; i < 50000; ++i) {
			s[static_cast<std::size_t>(i)] = static_cast<char>('!' + i % 94);
			t[static_cast<std::size_t>(i)] = static_cast<char>('!' + (i + 1) % 94);
		}
		test_is_isomorphic("n = 50000, 94-char cycle shifted", s, t, true);
	}
	{
		// same, but the very last char of t collides with another mapping
		std::string s(50000, ' ');
		std::string t(50000, ' ');
		for (int i{0}; i < 50000; ++i) {
			s[static_cast<std::size_t>(i)] = static_cast<char>('!' + i % 94);
			t[static_cast<std::size_t>(i)] = static_cast<char>('!' + (i + 1) % 94);
		}
		t.back() = t.front();
		test_is_isomorphic("n = 50000, 94-char cycle, collision at the end", s, t, false);
	}
	{
		// n = 50000, "ab" * 25000 -> "cd" * 25000
		std::string s;
		std::string t;
		for (int i{0}; i < 25000; ++i) {
			s += "ab";
			t += "cd";
		}
		test_is_isomorphic("n = 50000, two letters repeated", s, t, true);
		t.back() = 'c';
		test_is_isomorphic("n = 50000, two letters repeated, broken tail", s, t, false);
	}

	return 0;
}
```

Пришлите свою попытку, и я её разберу.

---

work 16:03 - 

---

egg & add => true
- (e)gg & (a)dd => (e)()() , {e: a}
-  e(g)g & a(d)d => (e)(g)() , {e: a, g: d}
-  eg(g) & ad(d) => (e)(g)(g) , {e: a, g: d} => true

foo & bar => false
- (f)oo & (b)ar => (f)()(), {f: b}
- f(o)o & b(a)r => (f)(o)(), {f: b, o: a}
- fo(o) & ba(r) => (f)(o)(o), {f: b, o: a} & o == r => false

foa & bao => true
- (f)oa & (b)ao => (f)()(), {f: b}
- f(o)a & b(a)o => (f)(o)(), {f: b, o: a}
- fo(a) & ba(o) => (f)(o)(a), {f: b, o: a, a: o}



---


```cpp
#include <iostream>
#include <format>
#include <string>
#include <cstddef>
#include <array>

namespace {
	constexpr int EMPTY{-1};
	constexpr std::size_t SIZE{256};

	bool is_isomorphic(const std::string& s,
					   const std::string& t) {
		const std::size_t size{s.size()};
		if (size != t.size()) return false;
		
		std::array<int, SIZE> s2t;
		s2t.fill(EMPTY);
		std::array<int, SIZE> t2s;
		t2s.fill(EMPTY);

		for (std::size_t i{0}; i < size; ++i) {
			const auto s_ch{static_cast<unsigned char>(s[i])};
			const auto t_ch{static_cast<unsigned char>(t[i])};

			if (t2s[t_ch] == EMPTY && s2t[s_ch] == EMPTY) {
				t2s[t_ch] = s_ch;
				s2t[s_ch] = t_ch;
			} else if (t2s[t_ch] != s_ch || s2t[s_ch] != t_ch) {
				return false;
			}
		}

		return true;
	}

	void test_is_isomorphic(const std::string& label,
							const std::string& s,
							const std::string& t,
							const bool expected) {
		const auto result{is_isomorphic(s, t)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_is_isomorphic("description example 1",
		"egg",
		"add",
		true);
	test_is_isomorphic("description example 2",
		"foo",
		"bar",
		false);
	test_is_isomorphic("description example 3",
		"paper",
		"title",
		true);
	test_is_isomorphic("single char, same",
		"a",
		"a",
		true);
	test_is_isomorphic("single char, different",
		"a",
		"b",
		true);
	test_is_isomorphic("two chars, swap",
		"ab",
		"ba",
		true);
	test_is_isomorphic("two chars, same to different",
		"aa",
		"ab",
		false);
	test_is_isomorphic("two chars, different to same",
		"ab",
		"aa",
		false);
	test_is_isomorphic("identical strings",
		"abcdef",
		"abcdef",
		true);
	test_is_isomorphic("two different map to one",
		"badc",
		"baba",
		false);
	test_is_isomorphic("one maps to two different",
		"baba",
		"badc",
		false);
	test_is_isomorphic("pattern repeats, isomorphic",
		"abab",
		"cdcd",
		true);
	test_is_isomorphic("pattern repeats, broken at the end",
		"abab",
		"cdce",
		false);
	test_is_isomorphic("pattern repeats, broken at the start",
		"abab",
		"ccdc",
		false);
	test_is_isomorphic("all same to all same",
		"aaaa",
		"zzzz",
		true);
	test_is_isomorphic("all same to one different at the end",
		"aaaa",
		"zzzy",
		false);
	test_is_isomorphic("mapping is a cycle",
		"abc",
		"bca",
		true);
	test_is_isomorphic("cycle with a repeated first char",
		"abca",
		"bcab",
		true);
	test_is_isomorphic("conflict only on the last char",
		"abcdefga",
		"bcdefgha",
		false);
	test_is_isomorphic("conflict only on the last char (reverse side)",
		"abcdefgh",
		"bcdefghb",
		false);
	test_is_isomorphic("digits and symbols",
		"1#1!",
		"a-a+",
		true);
	test_is_isomorphic("space and punctuation",
		"a b,a",
		"x y;x",
		true);
	test_is_isomorphic("space and punctuation, broken",
		"a b,a",
		"x y;z",
		false);
	test_is_isomorphic("uppercase and lowercase are different",
		"aA",
		"bb",
		false);
	test_is_isomorphic("uppercase and lowercase, isomorphic",
		"aA",
		"bc",
		true);
	test_is_isomorphic("whole alphabet to its reversal",
		"abcdefghijklmnopqrstuvwxyz",
		"zyxwvutsrqponmlkjihgfedcba",
		true);
	test_is_isomorphic("whole alphabet to itself shifted by one",
		"abcdefghijklmnopqrstuvwxyz",
		"bcdefghijklmnopqrstuvwxyza",
		true);
	test_is_isomorphic("whole alphabet doubled, 2 to 1 at the end",
		"abcdefghijklmnopqrstuvwxyza",
		"zyxwvutsrqponmlkjihgfedcbaz",
		true);
	test_is_isomorphic("random isomorphic 1, n=21",
		"hbhhdcbddcdfaefefbbbe",
		"wjwwvajvvavehrerejjjr",
		true);
	test_is_isomorphic("random isomorphic 2, n=8",
		"bfdebdhf",
		"qofpqfuo",
		true);
	test_is_isomorphic("random isomorphic 3, n=13",
		"bcaedfeffgagh",
		"cyqgfxgxxtqtp",
		true);
	test_is_isomorphic("random, two sources to one target 1, n=9",
		"dgcbdgfee",
		"egtsegtoo",
		false);
	test_is_isomorphic("random, two sources to one target 2, n=22",
		"fcheaaehagafegfedfbdce",
		"ygezppzepepyzeyzuyvugz",
		false);
	test_is_isomorphic("random, one source to two targets 1, n=13",
		"bcfeddbbadbac",
		"sobpaasswacwo",
		false);
	test_is_isomorphic("random, one source to two targets 2, n=10",
		"gagahgfhfa",
		"lplpdbadap",
		false);

	{
		// n = 50000, all 'a' -> all 'b'
		test_is_isomorphic("n = 50000, all equal",
			std::string(50000, 'a'),
			std::string(50000, 'b'),
			true);
	}
	{
		// n = 50000, all 'a' vs all 'b' except the last char
		std::string t(50000, 'b');
		t.back() = 'c';
		test_is_isomorphic("n = 50000, all equal vs broken tail",
			std::string(50000, 'a'),
			t,
			false);
	}
	{
		// n = 50000, 94 printable chars in a cycle -> shifted by one
		std::string s(50000, ' ');
		std::string t(50000, ' ');
		for (int i{0}; i < 50000; ++i) {
			s[static_cast<std::size_t>(i)] = static_cast<char>('!' + i % 94);
			t[static_cast<std::size_t>(i)] = static_cast<char>('!' + (i + 1) % 94);
		}
		test_is_isomorphic("n = 50000, 94-char cycle shifted", s, t, true);
	}
	{
		// same, but the very last char of t collides with another mapping
		std::string s(50000, ' ');
		std::string t(50000, ' ');
		for (int i{0}; i < 50000; ++i) {
			s[static_cast<std::size_t>(i)] = static_cast<char>('!' + i % 94);
			t[static_cast<std::size_t>(i)] = static_cast<char>('!' + (i + 1) % 94);
		}
		t.back() = t.front();
		test_is_isomorphic("n = 50000, 94-char cycle, collision at the end", s, t, false);
	}
	{
		// n = 50000, "ab" * 25000 -> "cd" * 25000
		std::string s;
		std::string t;
		for (int i{0}; i < 25000; ++i) {
			s += "ab";
			t += "cd";
		}
		test_is_isomorphic("n = 50000, two letters repeated", s, t, true);
		t.back() = 'c';
		test_is_isomorphic("n = 50000, two letters repeated, broken tail", s, t, false);
	}

	return 0;
}
```