#include "kth_largest_element.h"

#include <vector>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <format>

namespace kth_largest_element {
// solution 1
int find_kth_largest(const std::vector<int>& nums, const int k) {
    std::vector<int> copy{nums.begin(), nums.end()};
    std::ranges::sort(copy, std::greater<>());

    return copy[k - 1];
}

// solution 2
int find_kth_largest_heap(const std::vector<int>& nums, const int k) {
    std::priority_queue<int, std::vector<int>, std::greater<>> min_heap;

    for (const auto& num : nums) {
        min_heap.push(num);
        if (static_cast<int>(min_heap.size()) > k) {
            // выбрасываем наименьший, если размер превысил k
            min_heap.pop();
        }
    }

    return min_heap.top();
}

// solution 3

static int partition(std::vector<int>& nums,
                     const int left,
                     const int right,
                     const int pivot_index) {
    const int pivot_value{nums[pivot_index]};
    // прячем pivot в конец
    std::swap(nums[pivot_index], nums[right]);
    int store_index{left};

    for (int i{left}; i < right; ++i) {
        if (nums[i] < pivot_value) {
            std::swap(nums[i], nums[store_index]);
            ++store_index;
        }
    }

    // возвращаем pivot на финальную позицию
    std::swap(nums[store_index], nums[right]);
    return store_index;
}

static int quick_select(std::vector<int>& nums, const int left, const int right, const int target_index) {
    if (left == right) return nums[left];

    // случайный выбор pivot
    int pivot_index{left + std::rand() % (right - left + 1)};
    pivot_index = partition(nums, left, right, pivot_index);

    if (target_index == pivot_index) {
        return nums[target_index];
    }
    if (target_index < pivot_index) {
        return quick_select(nums, left, pivot_index - 1, target_index);
    }
    return quick_select(nums, pivot_index + 1, right, target_index);
}

int find_kth_largest_quick_select(const std::vector<int>& nums, const int k) {
    const int N{static_cast<int>(nums.size())};
    std::vector<int> copy{nums.begin(), nums.end()};

    return quick_select(copy, 0, N - 1, N - k);
}

void demo() {
    constexpr int K{2};
    const std::vector<int> NUMS{3,2,1,5,6,4};

    std::cout << std::format("solution 1: {}\n", find_kth_largest(NUMS, K));
    std::cout << std::format("solution 2: {}\n", find_kth_largest_heap(NUMS, K));
    std::cout << std::format("solution 3: {}\n", find_kth_largest_quick_select(NUMS, K));
}
}
