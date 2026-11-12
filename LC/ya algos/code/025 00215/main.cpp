#include <iostream>
#include <format>
#include <vector>
#include <string>
#include <cstddef>
#include <algorithm>
#include <random>
#include <queue>

namespace {
    constexpr int BAD_RESULT{-1};

    void print_vec(const std::string& label, const std::vector<int>& nums, std::size_t i0, std::size_t i1) {
        std::cout << std::format("{} ", label);
        for (std::size_t i{0}; i < nums.size(); ++i) {
            if (i == i0) std::cout << "[";
            if (i == i1) std::cout << "(";

            std::cout << nums[i];

            if (i == i1) std::cout << ")";
            if (i == i0) std::cout << "]";

            std::cout << ' ';
        }

        std::cout << std::endl;
    }

    int find_kth_largest(std::vector<int>& nums,
                         int k) {

        std::priority_queue<int, std::vector<int>, std::greater<>> pq;
        for (const auto num: nums) {
            pq.push(num);
            if (pq.size() > static_cast<std::size_t>(k)) {
                pq.pop();
            }
        }

\        return pq.size() != 0 ? pq.top() : BAD_RESULT;

        // std::mt19937 gen{42};
        // std::size_t left{0};
        // std::size_t right{nums.size()};
        // const std::size_t target{static_cast<std::size_t>(k-1)};
        // while (true) {
        //     const std::size_t pivot_idx{left + gen() % (right - left)};
        //     const int pivot{nums[pivot_idx]};
        //
        //     std::size_t lo{left};
        //     std::size_t i{left};
        //     std::size_t hi{right};
        //
        //     while (i < hi) {
        //         if (nums[i] > pivot) {
        //             print_vec("[before] nums[i] > pivot", nums, lo, i);
        //             std::swap(nums[lo++], nums[i++]);
        //             print_vec("[after] nums[i] > pivot", nums, lo-1, i-1);
        //         } else if (nums[i] == pivot) {
        //             ++i;
        //             std::cout << std::format("{} => {}\n", i - 1, i ) << std::flush;
        //         } else {
        //             print_vec("[before] nums[i] < pivot", nums, hi, i);
        //             std::swap(nums[i], nums[--hi]);
        //             print_vec("[after] nums[i] < pivot", nums, hi+1, i);
        //         }
        //     }
        //     if (target < lo) right = lo;
        //     else if (target < hi) return pivot;
        //     else left = hi;
        // }
        //
        // return BAD_RESULT;
    }

    void test_kth(const std::string& label,
                  std::vector<int> nums,
                  int k) {
        std::vector<int> sorted{nums};
        std::ranges::sort(sorted);
        const int expected{sorted[sorted.size() - static_cast<std::size_t>(k)]};

        const int result{find_kth_largest(nums, k)};

        std::cout << std::format("{}: {}\n",
            label,
            result == expected ? "OK" : "FAIL") << std::flush;
    }
}

int main(int argc, char *argv[]) {
    test_kth("description example 2",
        {3, 2, 3, 1, 2, 4, 5, 5, 6},
        4);

    return 0;
}
