#include <iostream>
#include <vector>
#include <deque>

namespace {
    std::vector<int> max_sliding_window(const std::vector<int>& nums, const int window_size) {
        std::deque<int> indexes;
        std::vector<int> result;

        if (window_size == 0) return result;

        for (int i{}; i< static_cast<int>(nums.size()); ++i) {
            while (!indexes.empty() && nums[indexes.back()] <= nums[i]) {
                indexes.pop_back();
            }
            indexes.push_back(i);

            if (indexes.front() <= i - window_size) {
                indexes.pop_front();
            }

            if (i >= window_size - 1) {
                result.push_back(nums[indexes.front()]);
            }
        }

        return result;
    }

    void print_vec(const std::vector<int>& vec, const std::string& label) {
        std::cout << std::format("[{}] (", label);
        for (int i{}; i < static_cast<int>(vec.size()); ++i) {
            if (i) std::cout << ',';
            std::cout << vec[i];

        }
        std::cout << ")\n";
    }

    void run_test(const std::string& desc,
                  const std::vector<int>& nums,
                  const int k,
                  const std::vector<int>& expected) {
        const auto result{max_sliding_window(nums, k)};
        const auto ok{result == expected};
        print_vec(result, desc);
        print_vec(expected, desc + " expected");
        std::cout << std::format(" -> {}\n", ok ? "OK" : "FAIL");
    }

}

int main() {
    run_test("Classical example", {1,3,-1,-3,5,3,6,7}, 3, {3,3,5,5,6,7});
    run_test("k=1 (single-element window)", {1,3,-1,-3,5}, 1, {1,3,-1,-3,5});
    run_test("k=n (a single window for the entire array)", {9,11,8,1,4}, 5, {11});
    run_test("Strictly decreasing array", {5,4,3,2,1}, 2, {5,4,3,2});
    run_test("Strictly increasing array", {1,2,3,4,5}, 2, {2,3,4,5});
    run_test("Recurring maxima", {4,4,4,4}, 2, {4,4,4});
    run_test("It is the same value, but the right kind of 'survivability' is required.", {1,3,3,5,5,3,3,1}, 3, {3,5,5,5,5,3});

    return 0;
}
