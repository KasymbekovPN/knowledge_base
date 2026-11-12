// 15:34 - 16:00

#include <cstddef>
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <array>
#include <algorithm>

namespace {
	constexpr std::size_t ALPHABET_SIZE{26};

	std::size_t get_index(const char ch) {
		return static_cast<std::size_t>(ch - 'a');
	}

	std::vector<int> find_anagrams(const std::string& s,
								   const std::string& p) {

		const std::size_t anagram_size{p.size()};
		const std::size_t string_size{s.size()};

		if (string_size < anagram_size) return {};

		std::array<int, ALPHABET_SIZE> anagram_mask{};
		std::array<int, ALPHABET_SIZE> window_mask{};
		for (std::size_t i{0}; i < anagram_size; ++i) {
			++anagram_mask[get_index(p[i])];
			++window_mask[get_index(s[i])];
		}

		std::vector<int> anagrams;
		if (anagram_mask == window_mask) anagrams.push_back(0);

		for (std::size_t i{anagram_size}; i < string_size; ++i) {
			--window_mask[get_index(s[i - anagram_size])];
			++window_mask[get_index(s[i])];

			if (anagram_mask != window_mask) continue;
			anagrams.push_back(static_cast<int>(i - anagram_size + 1));
		}

		return anagrams;
	}

	void test_find_anagrams(const std::string& label,
							const std::string& s,
							const std::string& p,
							std::vector<int> expected) {
		std::ranges::sort(expected);
		auto result{find_anagrams(s, p)};
		std::ranges::sort(result);

		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_find_anagrams("description example 1",
		"cbaebabacd",
		"abc",
		{0, 6});
	test_find_anagrams("description example 2",
		"abab",
		"ab",
		{0, 1, 2});
	test_find_anagrams("single char, match",
		"a",
		"a",
		{0});
	test_find_anagrams("single char, no match",
		"a",
		"b",
		{});
	test_find_anagrams("pattern longer than the string",
		"ab",
		"abc",
		{});
	test_find_anagrams("pattern equal to the string, anagram",
		"listen",
		"silent",
		{0});
	test_find_anagrams("pattern equal to the string, not anagram",
		"listen",
		"silenx",
		{});
	test_find_anagrams("pattern is a single letter, several matches",
		"abacada",
		"a",
		{0, 2, 4, 6});
	test_find_anagrams("pattern is a single letter, no match",
		"bcdef",
		"a",
		{});
	test_find_anagrams("all letters equal, overlapping matches",
		"aaaaaaaaaa",
		"aa",
		{0, 1, 2, 3, 4, 5, 6, 7, 8});
	test_find_anagrams("all letters equal, pattern longer than half",
		"aaaaaaaaaa",
		"aaaaaaaa",
		{0, 1, 2});
	test_find_anagrams("match at the very start",
		"abcxyz",
		"cab",
		{0});
	test_find_anagrams("match at the very end",
		"xyzabc",
		"cab",
		{3});
	test_find_anagrams("matches at both ends",
		"abcxabc",
		"bca",
		{0, 4});
	test_find_anagrams("same letters, different counts",
		"aab",
		"abb",
		{});
	test_find_anagrams("same set of letters, pattern has more of one",
		"aabbc",
		"abbc",
		{1});
	test_find_anagrams("pattern with repeated letters",
		"abaacbaaab",
		"aab",
		{0, 1, 5, 7});
	test_find_anagrams("no letters in common",
		"xyzxyzxyz",
		"abc",
		{});
	test_find_anagrams("window is almost an anagram",
		"abdabcabd",
		"abc",
		{3, 4, 5});
	test_find_anagrams("overlapping matches shifted by one",
		"abcabcabc",
		"abc",
		{0, 1, 2, 3, 4, 5, 6});
	test_find_anagrams("pattern contains a letter the string lacks",
		"aaaaaa",
		"aab",
		{});
	test_find_anagrams("string has the letters but too sparse",
		"axbxcx",
		"abc",
		{});
	test_find_anagrams("whole alphabet and its reversal",
		"abcdefghijklmnopqrstuvwxyz",
		"zyxwvutsrqponmlkjihgfedcba",
		{0});
	test_find_anagrams("whole alphabet doubled, windows of 26",
		"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
		"abcdefghijklmnopqrstuvwxyz",
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26});
	test_find_anagrams("pattern of two different letters",
		"abbabaab",
		"ab",
		{0, 2, 3, 4, 6});
	test_find_anagrams("random 1, n=18, m=2",
		"cbccbacaabbaaabaab",
		"aa",
		{7, 11, 12, 15});
	test_find_anagrams("random 2, n=22, m=5",
		"cabaabbaacbaacbbccbbbc",
		"abbcc",
		{12});
	test_find_anagrams("random 3, n=14, m=5",
		"acbccbbbabcaab",
		"ccaaa",
		{});
	test_find_anagrams("random 4, n=18, m=5",
		"bcbbacaabccaccaaba",
		"cbccb",
		{});
	test_find_anagrams("random 5, n=20, m=3",
		"cacbaaabccacbbbcaacc",
		"cba",
		{1, 2, 6, 10, 14});
	test_find_anagrams("random with planted anagrams 1, n=25, m=4",
		"ddfbjcdhbhbcecfeebhgfdeib",
		"efec",
		{12, 13});
	test_find_anagrams("random with planted anagrams 2, n=30, m=4",
		"hfejadbbfjbchfidifhiifdbiegjeg",
		"iidf",
		{13, 14, 19});

	{
		// s = 'a' * 30000, p = 'a' * 15000 -> every start 0..15000
		std::vector<int> expected;
		for (int i{0}; i <= 15000; ++i) expected.push_back(i);
		test_find_anagrams("n = 30000, all 'a', m = 15000",
			std::string(30000, 'a'),
			std::string(15000, 'a'),
			expected);
	}
	{
		// s = "ab" * 15000, p = "ab" -> every start 0..29998 ("ba" is an anagram too)
		std::string s;
		for (int i{0}; i < 15000; ++i) s += "ab";
		std::vector<int> expected;
		for (int i{0}; i <= 29998; ++i) expected.push_back(i);
		test_find_anagrams("n = 30000, \"ab\" repeated, p = \"ab\"",
			s,
			"ab",
			expected);
	}
	{
		// s = 'a' * 30000, p = "b" -> nothing
		test_find_anagrams("n = 30000, all 'a', p = \"b\"",
			std::string(30000, 'a'),
			"b",
			{});
	}
	{
		// s = abc...zabc...z..., p = the alphabet -> every window of 26 is a permutation of the alphabet
		std::string s(30000, 'a');
		for (std::size_t i{0}; i < s.size(); ++i) s[i] = static_cast<char>('a' + i % 26);
		std::vector<int> expected;
		for (int i{0}; i <= 30000 - 26; ++i) expected.push_back(i);
		test_find_anagrams("n = 30000, alphabet cycle, p = alphabet",
			s,
			"abcdefghijklmnopqrstuvwxyz",
			expected);
	}
	{
		// same string, p has 'y' twice and no 'z' -> nothing
		std::string s(30000, 'a');
		for (std::size_t i{0}; i < s.size(); ++i) s[i] = static_cast<char>('a' + i % 26);
		test_find_anagrams("n = 30000, alphabet cycle, p = almost the alphabet",
			s,
			"abcdefghijklmnopqrstuvwxyy",
			{});
	}
	{
		// s = 'a' * 15000 + 'b' * 15000, p = 'b' * 15000 + 'a' * 15000 -> only 0
		test_find_anagrams("n = 30000, p is a permutation of the whole string",
			std::string(15000, 'a') + std::string(15000, 'b'),
			std::string(15000, 'b') + std::string(15000, 'a'),
			{0});
	}
	{
		// p longer than s -> nothing
		test_find_anagrams("p = 30000, s = 1000",
			std::string(1000, 'a'),
			std::string(30000, 'a'),
			{});
	}
	{
		// s = 'a' * 30000, p = 'a' * 14999 + 'b' -> nothing
		test_find_anagrams("n = 30000, m = 15000, one wrong letter",
			std::string(30000, 'a'),
			std::string(14999, 'a') + 'b',
			{});
	}
	{
		// s = 'a' * 29999 + 'b', p = 'a' * 14999 + 'b' -> only the last window, start 15000
		test_find_anagrams("n = 30000, m = 15000, match only at the end",
			std::string(29999, 'a') + 'b',
			std::string(14999, 'a') + 'b',
			{15000});
	}

	return 0;
}