// 15:18 - 15:32

#include <iostream>
#include <format>
#include <string>
#include <cstddef>
#include <array>

namespace {
	constexpr int EMPTY{-1};
	constexpr std::size_t ALPHABET_SIZE{256};

	bool is_isomorphic(const std::string& s,
					   const std::string& t) {

		const std::size_t size{s.size()};
		if (size != t.size()) return false;

		std::array<int, ALPHABET_SIZE> s_array{};
		s_array.fill(EMPTY);
		std::array<int, ALPHABET_SIZE> t_array{};
		t_array.fill(EMPTY);

		for (std::size_t i{0}; i < size; ++i) {
			const auto s_ch{static_cast<unsigned char>(s[i])};
			const auto t_ch{static_cast<unsigned char>(t[i])};

			if (s_array[s_ch] == EMPTY && t_array[t_ch] == EMPTY) {
				s_array[s_ch] = t_ch;
				t_array[t_ch] = s_ch;
			} else if (s_array[s_ch] != t_ch || t_array[t_ch] != s_ch) {
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