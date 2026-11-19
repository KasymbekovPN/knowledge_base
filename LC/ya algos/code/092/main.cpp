#include <iostream>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>
#include <chrono>
#include <algorithm>

namespace {

int search(const std::vector<int>& nums, int lo, int hi, int target ) {
    if (lo > hi) return -1;
    const auto lo_value{nums[static_cast<std::size_t>(lo)]};
    const auto hi_value{nums[static_cast<std::size_t>(hi)]};
    if (lo_value <= hi_value &&
        (target < lo_value || target > hi_value)) return -1;

    const auto mi{lo + (hi - lo) / 2};
    if (nums[mi] == target) return mi;

    const auto result = search(nums, lo, mi - 1, target);
    return result == -1 ? search(nums, mi + 1, hi, target) : result;
}

// nums was sorted in ascending order (all values distinct) and then rotated:
// {0, 1, 2, 4, 5, 6, 7} could become {4, 5, 6, 7, 0, 1, 2}. The rotation may be zero.
// Return the index of target in nums, or -1 if it is not there.
int search_rotated(const std::vector<int>& nums, int target) {
	const auto size{nums.size()};
	if (size == 0) { return -1; }

    return search(nums, 0, static_cast<int>(size) - 1, target);
}

// Checker: linear scan.
int linear_search(const std::vector<int>& nums, int target) {
    for (std::size_t i = 0; i < nums.size(); ++i)
        if (nums[i] == target) return static_cast<int>(i);
    return -1;
}

void report(const std::string& label, bool ok) {
    std::cout << label << ": " << (ok ? "OK" : "FAIL") << '\n';
}

void test_search(const std::string& label, const std::vector<int>& nums, int target) {
    report(label, search_rotated(nums, target) == linear_search(nums, target));
}

// Checks every element of nums and several values that are not in it.
void test_every(const std::string& label, const std::vector<int>& nums) {
    bool ok = true;
    for (int target : nums) ok = ok && search_rotated(nums, target) == linear_search(nums, target);
    const int low = nums.empty() ? 0 : *std::min_element(nums.begin(), nums.end());
    const int high = nums.empty() ? 0 : *std::max_element(nums.begin(), nums.end());
    for (int target : {low - 1, high + 1, low + 1, high - 1})
        ok = ok && search_rotated(nums, target) == linear_search(nums, target);
    report(label, ok);
}

// Sorted values 2*i + 1 rotated left by `shift`.
std::vector<int> rotated(std::size_t n, std::size_t shift) {
    std::vector<int> sorted(n);
    for (std::size_t i = 0; i < n; ++i) sorted[i] = static_cast<int>(2 * i) + 1;
    std::rotate(sorted.begin(), sorted.begin() + static_cast<std::ptrdiff_t>(shift % (n == 0 ? 1 : n)), sorted.end());
    return sorted;
}

void test_big(const std::string& label, const std::vector<int>& nums) {
    // 200000 queries: present values and absent values (even numbers are never in the array).
    std::vector<int> queries;
    unsigned x = 777u;
    for (int i = 0; i < 200000; ++i) {
        x = x * 1103515245u + 12345u;
        const std::size_t pos = (x >> 8) % nums.size();
        queries.push_back(i % 2 == 0 ? nums[pos] : nums[pos] + 1);
    }
    std::vector<int> expected;
    {
        // two-pointer free reference: map value -> index
        std::vector<std::pair<int, int>> sorted_values;
        for (std::size_t i = 0; i < nums.size(); ++i) sorted_values.push_back({nums[i], static_cast<int>(i)});
        std::sort(sorted_values.begin(), sorted_values.end());
        for (int q : queries) {
            const auto it = std::lower_bound(sorted_values.begin(), sorted_values.end(), std::make_pair(q, -1));
            expected.push_back(it != sorted_values.end() && it->first == q ? it->second : -1);
        }
    }
    std::vector<int> got;
    got.reserve(queries.size());
    const auto start = std::chrono::steady_clock::now();
    for (int q : queries) got.push_back(search_rotated(nums, q));
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const bool fast = elapsed < std::chrono::seconds(1);
    report(label + (fast ? "" : " (too slow)"), got == expected && fast);
}

void test_all() {
    test_search("description example 1",
        {4, 5, 6, 7, 0, 1, 2}, 0);
    test_search("description example 2: not found",
        {4, 5, 6, 7, 0, 1, 2}, 3);
    test_search("one element, found",
        {1}, 1);
    test_search("one element, not found",
        {1}, 0);
    test_search("empty",
        {}, 5);
    test_search("two elements, rotated, first",
        {3, 1}, 3);
    test_search("two elements, rotated, second",
        {3, 1}, 1);
    test_search("two elements, not rotated",
        {1, 3}, 3);
    test_search("two elements, not found",
        {3, 1}, 2);
    test_search("not rotated",
        {1, 2, 3, 4, 5, 6, 7}, 5);
    test_search("rotated by one",
        {7, 1, 2, 3, 4, 5, 6}, 7);
    test_search("rotated by n - 1",
        {2, 3, 4, 5, 6, 7, 1}, 1);
    test_search("target in the left part",
        {6, 7, 8, 1, 2, 3, 4, 5}, 7);
    test_search("target in the right part",
        {6, 7, 8, 1, 2, 3, 4, 5}, 3);
    test_search("target is the pivot",
        {6, 7, 8, 1, 2, 3, 4, 5}, 1);
    test_search("target is the maximum",
        {6, 7, 8, 1, 2, 3, 4, 5}, 8);
    test_search("target smaller than everything",
        {6, 7, 8, 1, 2, 3, 4, 5}, 0);
    test_search("target larger than everything",
        {6, 7, 8, 1, 2, 3, 4, 5}, 9);
    test_search("negative values",
        {-3, -2, -1, -9, -8, -7, -5}, -8);
    test_search("extreme values",
        {0, 2147483647, -2147483647 - 1, -5}, -2147483647 - 1);
    test_every("every element: size 3, all rotations",
        rotated(3, 1));
    test_every("every element: size 7, rotation 3",
        rotated(7, 3));
    test_every("every element: size 8, rotation 5",
        rotated(8, 5));
    test_every("every element: size 10, rotation 9",
        rotated(10, 9));
    test_every("every element: size 15, rotation 1",
        rotated(15, 1));
    test_every("every element: size 16, rotation 0",
        rotated(16, 0));
    test_every("every element: size 31, rotation 20",
        rotated(31, 20));

    // ---- big tests ----
    test_big("big: 1000000 elements, rotation 0, 200000 queries", rotated(1000000, 0));
    test_big("big: 1000000 elements, rotation 1, 200000 queries", rotated(1000000, 1));
    test_big("big: 1000000 elements, rotation 500000, 200000 queries", rotated(1000000, 500000));
    test_big("big: 1000000 elements, rotation 999999, 200000 queries", rotated(1000000, 999999));
    test_big("big: 1000000 elements, rotation 123457, 200000 queries", rotated(1000000, 123457));
}

}  // namespace

int main() {
    test_all();
    return 0;
}