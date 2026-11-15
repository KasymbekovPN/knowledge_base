#include <iostream>
#include <cstddef>
#include <string>
#include <set>
#include <vector>
#include <chrono>
#include <algorithm>

namespace {

// Return all unique triplets {a, b, c} (taken from three different positions of nums)
// with a + b + c == 0. The order of triplets and the order inside a triplet do not matter.
// Result must not contain duplicate triplets.
std::vector<std::vector<int>> three_sum(std::vector<int> nums) {
	std::ranges::sort(nums);

	const int size{static_cast<int>(nums.size())};
	std::vector<std::vector<int>> result;
	for (int i{0}; i < size - 2; ++i) {
		if (i > 0 && nums[i] == nums[i - 1]) { continue; }
		
		int lo{i + 1};
		int hi{size - 1};
		while (lo < hi) {
			const int sum{nums[i] + nums[lo] + nums[hi]};
			if (sum == 0) {
				std::vector<int> buffer{nums[i], nums[lo], nums[hi]};
				result.push_back(std::move(buffer));
				
				++lo;
				while (lo < hi && nums[lo] == nums[lo - 1]) { ++lo; }
				
				--hi;
				while (lo < hi && nums[hi] == nums[hi + 1]) { --hi; }
			} 
			else if (sum < 0) { ++lo; }
			else { --hi; }
		}
	}

    return result;
}

// Checker: brute force O(n^3) for small tests, two pointers for big ones.
std::vector<std::vector<int>> brute_force(const std::vector<int>& nums) {
    std::set<std::vector<int>> found;
    for (std::size_t i = 0; i < nums.size(); ++i)
        for (std::size_t j = i + 1; j < nums.size(); ++j)
            for (std::size_t k = j + 1; k < nums.size(); ++k)
                if (nums[i] + nums[j] + nums[k] == 0) {
                    std::vector<int> t{nums[i], nums[j], nums[k]};
                    std::sort(t.begin(), t.end());
                    found.insert(t);
                }
    return std::vector<std::vector<int>>(found.begin(), found.end());
}

std::vector<std::vector<int>> two_pointers(std::vector<int> nums) {
    std::sort(nums.begin(), nums.end());
    std::vector<std::vector<int>> result;
    const int n = static_cast<int>(nums.size());
    for (int i = 0; i + 2 < n; ++i) {
        if (nums[static_cast<std::size_t>(i)] > 0) break;
        if (i > 0 && nums[static_cast<std::size_t>(i)] == nums[static_cast<std::size_t>(i - 1)]) continue;
        int lo = i + 1, hi = n - 1;
        while (lo < hi) {
            const long long s = static_cast<long long>(nums[static_cast<std::size_t>(i)]) +
                                nums[static_cast<std::size_t>(lo)] + nums[static_cast<std::size_t>(hi)];
            if (s < 0) ++lo;
            else if (s > 0) --hi;
            else {
                result.push_back({nums[static_cast<std::size_t>(i)], nums[static_cast<std::size_t>(lo)],
                                  nums[static_cast<std::size_t>(hi)]});
                ++lo; --hi;
                while (lo < hi && nums[static_cast<std::size_t>(lo)] == nums[static_cast<std::size_t>(lo - 1)]) ++lo;
                while (lo < hi && nums[static_cast<std::size_t>(hi)] == nums[static_cast<std::size_t>(hi + 1)]) --hi;
            }
        }
    }
    return result;
}

std::vector<std::vector<int>> normalize(std::vector<std::vector<int>> triplets) {
    for (auto& t : triplets) std::sort(t.begin(), t.end());
    std::sort(triplets.begin(), triplets.end());
    return triplets;
}

void report(const std::string& label, bool ok) {
    std::cout << label << ": " << (ok ? "OK" : "FAIL") << '\n';
}

void test_three_sum(const std::string& label, const std::vector<int>& nums) {
    const auto got = normalize(three_sum(nums));
    report(label, got == brute_force(nums));
}

void test_big(const std::string& label, const std::vector<int>& nums) {
    const auto expected = normalize(two_pointers(nums));
    const auto start = std::chrono::steady_clock::now();
    const auto got = normalize(three_sum(nums));
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const bool fast = elapsed < std::chrono::seconds(1);
    report(label + (fast ? "" : " (too slow)"), got == expected && fast);
}

void test_all() {
    test_three_sum("description example 1",
        {-1, 0, 1, 2, -1, -4});
    test_three_sum("description example 2: no triplets",
        {0, 1, 1});
    test_three_sum("description example 3: zeros",
        {0, 0, 0});
    test_three_sum("fewer than three elements",
        {0, 0});
    test_three_sum("empty",
        {});
    test_three_sum("four zeros: one triplet",
        {0, 0, 0, 0});
    test_three_sum("all positive",
        {1, 2, 3, 4, 5});
    test_three_sum("all negative",
        {-5, -4, -3, -2, -1});
    test_three_sum("one zero only",
        {0, 1, -1});
    test_three_sum("zero with duplicates of a pair",
        {0, 1, -1, 1, -1, 0});
    test_three_sum("same value used three times is not allowed",
        {2, -1, -1, 5});
    test_three_sum("value used twice is allowed",
        {-2, 1, 1, 7});
    test_three_sum("many duplicates",
        {-2, -2, -2, 0, 0, 0, 2, 2, 2, 4, 4, -4, -4});
    test_three_sum("already sorted",
        {-4, -1, -1, 0, 1, 2});
    test_three_sum("reverse sorted",
        {2, 1, 0, -1, -1, -4});
    test_three_sum("extreme values",
        {-100000, 100000, 0, -100000, 100000, 50000, -50000});
    test_three_sum("two triplets share two numbers",
        {-3, 1, 2, 1, -3, 2, 0, 3, -2, -1});
    test_three_sum("random 1: 8 elements from -4 to 4",
        {3, -4, 1, 0, -2, 4, -1, 2});
    test_three_sum("random 2: 12 elements from -5 to 5",
        {-5, 2, 3, 0, 0, -1, 4, -4, 1, -3, 5, -2});
    test_three_sum("random 3: 20 elements from -3 to 3",
        {1, -3, 0, 2, 2, -1, -2, 3, 0, 1, -3, -1, 2, 0, -2, 3, 1, -3, 0, 2});
    test_three_sum("random 4: 30 elements from -10 to 10",
        {7, -3, 0, 9, -10, 4, 2, -6, 8, 1, -1, 5, -9, 3, -4, 10, -2, 6, -7, -8, 0, 7, -5, 2, 9, -3, 1, -6, 4, -10});

    // ---- big tests ----
    {
        std::vector<int> nums(3000, 0);
        test_big("big: 3000 zeros", nums);
    }
    {
        std::vector<int> nums(3000);
        for (std::size_t i = 0; i < nums.size(); ++i) nums[i] = static_cast<int>(i % 2 == 0 ? i / 2 + 1 : -(i / 2 + 1));
        test_big("big: 3000 symmetric values", nums);
    }
    {
        std::vector<int> nums(3000);
        unsigned x = 12345u;
        for (auto& v : nums) { x = x * 1103515245u + 12345u; v = static_cast<int>((x >> 8) % 200001u) - 100000; }
        test_big("big: 3000 pseudo-random values from -100000 to 100000", nums);
    }
    {
        std::vector<int> nums(3000);
        unsigned x = 777u;
        for (auto& v : nums) { x = x * 1103515245u + 12345u; v = static_cast<int>((x >> 8) % 41u) - 20; }
        test_big("big: 3000 pseudo-random values from -20 to 20 (many duplicates)", nums);
    }
    {
        std::vector<int> nums(3000, 1);
        test_big("big: 3000 equal positive values", nums);
    }
}

}  // namespace

int main() {
    test_all();
    return 0;
}