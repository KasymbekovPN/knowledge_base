#include "search_rotated_sorted.h"

#include <vector>
#include <iostream>
#include <format>

namespace search_rotated_sorted {

int search(const std::vector<int>& nums, const int target) {
    int left{};
    int right{static_cast<int>(nums.size()) - 1};

    while (left <= right) {
        const int mid{left + (right - left) / 2};

        if (nums[mid] == target) return mid;

        if (nums[left] <= nums[mid]) {
            // левая половина [left, mid] отсортирована
            if (nums[left] <= target && target < nums[mid]) {
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        } else {
            // правая половина [mid, right] отсортирована
            if (nums[mid] < target && target <= nums[right]) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
    }

    return BAD_RESULT;
}

void demo() {
    constexpr int BAD_TARGET{3};
    constexpr int GOOD_TARGET{0};
    const std::vector<int> NUMS{4,5,6,7,0,1,2};

    std::cout << std::format("target: {} => {}\n", BAD_TARGET, search(NUMS, BAD_TARGET));
    std::cout << std::format("target: {} => {}\n", GOOD_TARGET, search(NUMS, GOOD_TARGET));
}

}
