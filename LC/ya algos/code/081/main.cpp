#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include <chrono>

namespace {

std::size_t get_id_(const int id) {
	return static_cast<std::size_t>(id);
}

// result[i] is the product of all elements of nums except nums[i].
// Do not use division. All products fit into int. There are at least 2 elements.
std::vector<int> product_except_self(const std::vector<int>& nums) {
	const int size{static_cast<int>(nums.size())};
	if (size < 2) return {};

	std::vector<int> result(size, 1);
	for (int i{1}; i < size; ++i) {
		result[get_id_(i)] = result[get_id_(i-1)] * nums[get_id_(i-1)];
	}
	
	int suf{1};
	for (int i{size - 1}; i >= 0; --i) {
		result[i] *= suf;
		suf *= nums[get_id_(i)];
	}

    return result;
}

// Reference with a naive O(n^2) loop (small tests) and a two-pass O(n) version (big tests).
std::vector<int> naive(const std::vector<int>& nums) {
    std::vector<int> result(nums.size(), 1);
    for (std::size_t i = 0; i < nums.size(); ++i)
        for (std::size_t j = 0; j < nums.size(); ++j)
            if (i != j) result[i] *= nums[j];
    return result;
}

std::vector<int> two_pass(const std::vector<int>& nums) {
    std::vector<int> result(nums.size(), 1);
    int prefix = 1;
    for (std::size_t i = 0; i < nums.size(); ++i) { result[i] = prefix; prefix *= nums[i]; }
    int suffix = 1;
    for (std::size_t i = nums.size(); i-- > 0;) { result[i] *= suffix; suffix *= nums[i]; }
    return result;
}

void report(const std::string& label, bool ok) {
    std::cout << label << ": " << (ok ? "OK" : "FAIL") << '\n';
}

void test_product(const std::string& label, std::vector<int> nums) {
    const std::vector<int> copy = nums;
    const bool ok = product_except_self(nums) == naive(nums) && nums == copy;
    report(label, ok);
}

void test_big(const std::string& label, std::vector<int> nums) {
    const std::vector<int> expected = two_pass(nums);
    const auto start = std::chrono::steady_clock::now();
    const std::vector<int> got = product_except_self(nums);
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const bool fast = elapsed < std::chrono::seconds(1);
    report(label + (fast ? "" : " (too slow)"), got == expected && fast);
}

void test_all() {
	test_product("description example 1",
		{1, 2, 3, 4});
	test_product("description example 2",
		{-1, 1, 0, -3, 3});
	test_product("two elements",
		{3, 5});
	test_product("two elements, with a zero",
		{0, 5});
	test_product("two zeros in the middle",
		{1, 0, 2, 0, 3});
	test_product("only zeros",
		{0, 0, 0});
	test_product("one zero at the start",
		{0, 2, 3, 4});
	test_product("one zero at the end",
		{2, 3, 4, 0});
	test_product("all ones",
		{1, 1, 1, 1, 1});
	test_product("all minus ones",
		{-1, -1, -1, -1});
	test_product("negative values",
		{-2, 3, -4, 5});
	test_product("one negative value",
		{2, -3, 4});
	test_product("large values that fit the answer",
		{46340, 46340});
	test_product("two elements, extreme product",
		{-46340, 46340});
	test_product("a one in the middle",
		{2, 1, 3});
	test_product("ascending",
		{1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
	test_product("zero and negatives",
		{-5, 0, -2, 3});
	test_product("random 1: 8 elements from -3 to 3",
		{-2, 0, 2, 0, 2, -1, 0, 3});
	test_product("random 2: 10 elements from -2 to 2",
		{-1, 1, 2, -1, 2, 2, -1, -2, -2, 0});
	test_product("random 3: 15 elements from -3 to 3",
		{1, 0, -3, -2, 3, 3, -2, -2, 2, -3, 0, 2, 3, 0, 1});
	test_product("random 4: 30 elements from -1 to 1",
		{0, -1, 0, 1, 0, 1, -1, 1, 0, -1, -1, 1, -1, -1, -1, 0, -1, 1, 1, 0, 0, 0, -1, -1, 1, -1, 0, -1, -1, -1});
	test_product("random 5: 60 elements from 0 to 2",
		{0, 2, 2, 2, 2, 0, 0, 2, 1, 2, 1, 0, 1, 0, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 0, 0, 0, 2, 2, 1, 0, 1, 0, 2, 2, 2, 2, 0, 2, 2, 1, 2, 2, 0, 0, 2, 0, 2, 1, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1});

	// ---- big tests ----
	{
		std::vector<int> nums(1000000, 1);
		test_big("big: 1000000 ones", nums);
	}
	{
		std::vector<int> nums(1000000, 1);
		nums[123456] = 0;
		test_big("big: 1000000 ones and one zero", nums);
	}
	{
		std::vector<int> nums(1000000, 1);
		nums[10] = 0; nums[999990] = 0;
		test_big("big: 1000000 ones and two zeros", nums);
	}
	{
		std::vector<int> nums(1000000, 1);
		for (std::size_t i = 0; i < 20; ++i) nums[i * 50000] = (i % 2 == 0) ? -2 : 2;
		nums[0] = 1;
		test_big("big: 1000000 ones with a few twos", nums);
	}
	{
		std::vector<int> nums(1000000);
		for (std::size_t i = 0; i < nums.size(); ++i) nums[i] = (i % 2 == 0) ? 1 : -1;
		test_big("big: 1000000 alternating signs", nums);
	}
}

}  // namespace

int main() {
	test_all();
	return 0;
}