#include <iostream>
#include <format>
#include <cstddef>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

namespace {
	int search_rotated(const std::vector<int>& nums,
					   int target) {
		const std::size_t size{nums.size()};
		if (size == 0) return -1;

		const std::function<int(int, int)> search_in_range = [&](int lo,
													  int hi) -> int {
			if (lo > hi) return -1;
			const auto left_value{nums[static_cast<std::size_t>(lo)]};
			const auto right_value{nums[static_cast<std::size_t>(hi)]};
			if (left_value <= right_value &&
				(target < left_value || target > right_value)) return -1;

			const auto mid_idx{lo + (hi - lo) / 2};
			if (nums[mid_idx] == target) return mid_idx;

			const auto result = search_in_range(lo, mid_idx - 1);
			return result == -1 ? search_in_range(mid_idx + 1, hi) : result;
		};

		return search_in_range(0, static_cast<int>(size - 1));
	}

	int expected_index(const std::vector<int>& nums, int target) {
		for (std::size_t i{0}; i < nums.size(); ++i) {
			if (nums[i] == target) return static_cast<int>(i);
		}
		return -1;
	}

	void test_search(const std::string& label,
					 const std::vector<int>& nums,
					 int target) {
		const auto expected{expected_index(nums, target)};

		const auto result{search_rotated(nums, target)};

		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}

	// many queries against one array: a linear scan cannot pass this
	void test_search_many(const std::string& label,
						  const std::vector<int>& nums,
						  const std::vector<int>& targets) {
		std::vector<std::pair<int, int>> sorted;
		sorted.reserve(nums.size());
		for (std::size_t i{0}; i < nums.size(); ++i) {
			sorted.emplace_back(nums[i], static_cast<int>(i));
		}
		std::ranges::sort(sorted);

		bool ok{true};
		for (const int target : targets) {
			const auto it{std::ranges::lower_bound(sorted, std::pair{target, -1})};
			const int expected{it != sorted.end() && it->first == target ? it->second : -1};
			if (search_rotated(nums, target) != expected) {
				ok = false;
				break;
			}
		}

		std::cout << std::format("{}: {}\n",
			label,
			ok ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_search("description example 1",
		{4, 5, 6, 7, 0, 1, 2},
		0);
	test_search("description example 2",
		{4, 5, 6, 7, 0, 1, 2},
		3);
	test_search("description example 3",
		{1},
		0);
	test_search("single element, found",
		{1},
		1);
	test_search("two elements, not rotated, first",
		{1, 3},
		1);
	test_search("two elements, not rotated, second",
		{1, 3},
		3);
	test_search("two elements, rotated, first",
		{3, 1},
		3);
	test_search("two elements, rotated, second",
		{3, 1},
		1);
	test_search("two elements, missing",
		{3, 1},
		2);
	test_search("three elements, rotated by 1",
		{3, 1, 2},
		1);
	test_search("three elements, rotated by 2",
		{2, 3, 1},
		1);
	test_search("not rotated, found in the middle",
		{1, 2, 3, 4, 5, 6, 7},
		4);
	test_search("not rotated, first",
		{1, 2, 3, 4, 5, 6, 7},
		1);
	test_search("not rotated, last",
		{1, 2, 3, 4, 5, 6, 7},
		7);
	test_search("not rotated, missing below",
		{2, 3, 4, 5, 6, 7, 8},
		1);
	test_search("not rotated, missing above",
		{2, 3, 4, 5, 6, 7, 8},
		9);
	test_search("rotated by 1, target is the minimum",
		{7, 1, 2, 3, 4, 5, 6},
		1);
	test_search("rotated by 1, target is the maximum",
		{7, 1, 2, 3, 4, 5, 6},
		7);
	test_search("rotated by n - 1, target is the maximum",
		{2, 3, 4, 5, 6, 7, 1},
		7);
	test_search("rotated by n - 1, target is the minimum",
		{2, 3, 4, 5, 6, 7, 1},
		1);
	test_search("target is the first element",
		{5, 6, 7, 1, 2, 3, 4},
		5);
	test_search("target is the last element",
		{5, 6, 7, 1, 2, 3, 4},
		4);
	test_search("target is the pivot (the minimum)",
		{5, 6, 7, 1, 2, 3, 4},
		1);
	test_search("target is just before the pivot",
		{5, 6, 7, 1, 2, 3, 4},
		7);
	test_search("target in the left part",
		{5, 6, 7, 1, 2, 3, 4},
		6);
	test_search("target in the right part",
		{5, 6, 7, 1, 2, 3, 4},
		3);
	test_search("missing between the parts",
		{5, 6, 8, 1, 2, 3, 4},
		7);
	test_search("missing below the minimum",
		{5, 6, 7, 8, 9, 3, 4},
		2);
	test_search("missing above the maximum",
		{5, 6, 7, 8, 9, 3, 4},
		10);
	test_search("negative values, rotated",
		{-3, -2, -1, -9, -8, -7, -5},
		-8);
	test_search("negative values, missing",
		{-3, -2, -1, -9, -8, -7, -5},
		-4);
	test_search("mixed signs, rotated",
		{2, 5, 9, -6, -3, 0, 1},
		-3);
	test_search("mixed signs, target zero",
		{2, 5, 9, -6, -3, 0, 1},
		0);
	test_search("extreme values, minimum",
		{0, 1000000000, -1000000000, -5},
		-1000000000);
	test_search("extreme values, maximum",
		{0, 1000000000, -1000000000, -5},
		1000000000);
	test_search("extreme values, missing",
		{1000000000, -1000000000},
		0);
	test_search("extreme values, unrotated",
		{-1000000000, 0, 1000000000},
		1000000000);
	test_search("even length, pivot in the middle",
		{5, 6, 7, 8, 1, 2, 3, 4},
		2);
	test_search("even length, target in the left part",
		{5, 6, 7, 8, 1, 2, 3, 4},
		8);
	test_search("odd length, pivot in the middle",
		{4, 5, 6, 7, 1, 2, 3},
		6);
	test_search("random 1, n=11, rotation=4",
		{0, 10, 12, 13, 14, 17, 19, -10, -9, -6, -3},
		19);
	test_search("random 2, n=3, rotation=1",
		{6, 13, -1},
		17);
	test_search("random 3, n=13, rotation=5",
		{-3, -2, 0, 3, 7, 8, 15, 18, -18, -17, -13, -7, -5},
		8);
	test_search("random 4, n=14, rotation=5",
		{-3, -1, 0, 1, 2, 4, 5, 12, 13, -20, -19, -17, -15, -5},
		14);
	test_search("random 5, n=3, rotation=0",
		{-16, -15, -11},
		-21);
	test_search("random 6, n=2, rotation=1",
		{6, -3},
		6);
	test_search("random 7, n=12, rotation=6",
		{9, 10, 11, 14, 18, 19, -20, -16, -5, -1, 2, 4},
		19);
	test_search("random 8, n=3, rotation=2",
		{14, -20, -5},
		-20);

	{
		// n = 1000000, distinct even numbers rotated by 123457, 200000 queries
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = 2 * i - 1000000;
		std::ranges::rotate(nums, nums.begin() + 123457);
		std::vector<int> targets;
		unsigned state{33};
		for (int i{0}; i < 200000; ++i) {
			state = state * 1103515245u + 12345u;
			targets.push_back(static_cast<int>((state >> 4) % 2100000u) - 1050000);
		}
		test_search_many("n = 1000000, rotated by 123457, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, not rotated, 200000 queries
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = 3 * i - 1500000;
		std::vector<int> targets;
		unsigned state{5};
		for (int i{0}; i < 200000; ++i) {
			state = state * 1103515245u + 12345u;
			targets.push_back(static_cast<int>((state >> 4) % 3100000u) - 1550000);
		}
		test_search_many("n = 1000000, not rotated, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, rotated by 1, 200000 queries
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i;
		std::ranges::rotate(nums, nums.begin() + 1);
		std::vector<int> targets;
		for (int i{0}; i < 200000; ++i) targets.push_back((i * 7) % 1000100 - 50);
		test_search_many("n = 1000000, rotated by 1, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, rotated by n - 1, 200000 queries
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = i;
		std::ranges::rotate(nums, nums.begin() + 999999);
		std::vector<int> targets;
		for (int i{0}; i < 200000; ++i) targets.push_back((i * 7) % 1000100 - 50);
		test_search_many("n = 1000000, rotated by n - 1, 200000 queries", nums, targets);
	}
	{
		// n = 1000000, extreme values around the pivot
		std::vector<int> nums(1000000);
		for (int i{0}; i < 1000000; ++i) nums[static_cast<std::size_t>(i)] = -1000000000 + 2000 * i;
		std::ranges::rotate(nums, nums.begin() + 500000);
		test_search("n = 1000000, target is the minimum", nums, -1000000000);
		test_search("n = 1000000, target is the maximum", nums, -1000000000 + 2000 * 999999);
		test_search("n = 1000000, target is missing", nums, 1000000000);
	}

	return 0;
}
