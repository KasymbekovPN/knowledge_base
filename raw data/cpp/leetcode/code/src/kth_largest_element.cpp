#include "kth_largest_element.h"

#include <vector>

namespace kth_largest_element {
// solution 1
int find_kth_largest(const std::vector<int>& nums, const int k) {
    //<
// #include <vector>
// #include <algorithm>
//
//     int findKthLargest(std::vector<int> nums, int k) {
//         std::sort(nums.begin(), nums.end(), std::greater<int>());
//         return nums[k - 1];
//     }
}

// solution 2
int find_kth_largest_heap(const std::vector<int>& nums, const int k) {
// #include <vector>
// #include <queue>
//
//     int findKthLargestHeap(const std::vector<int>& nums, int k) {
//         std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;
//
//         for (int num : nums) {
//             minHeap.push(num);
//             if (static_cast<int>(minHeap.size()) > k) {
//                 minHeap.pop(); // выбрасываем наименьший, если размер превысил k
//             }
//         }
//
//         return minHeap.top();
//     }
}

// solution 3
int find_kth_largest_quick_select(const std::vector<int>& nums, const int k) {
// #include <vector>
// #include <cstdlib>
//
//     int partition(std::vector<int>& nums, int left, int right, int pivotIndex) {
//         int pivotValue = nums[pivotIndex];
//         std::swap(nums[pivotIndex], nums[right]); // прячем pivot в конец
//         int storeIndex = left;
//
//         for (int i = left; i < right; ++i) {
//             if (nums[i] < pivotValue) {
//                 std::swap(nums[i], nums[storeIndex]);
//                 ++storeIndex;
//             }
//         }
//
//         std::swap(nums[storeIndex], nums[right]); // возвращаем pivot на финальную позицию
//         return storeIndex;
//     }
//
//     int quickSelect(std::vector<int>& nums, int left, int right, int targetIndex) {
//         if (left == right) {
//             return nums[left];
//         }
//
//         int pivotIndex = left + std::rand() % (right - left + 1); // случайный выбор pivot
//         pivotIndex = partition(nums, left, right, pivotIndex);
//
//         if (targetIndex == pivotIndex) {
//             return nums[targetIndex];
//         } else if (targetIndex < pivotIndex) {
//             return quickSelect(nums, left, pivotIndex - 1, targetIndex);
//         } else {
//             return quickSelect(nums, pivotIndex + 1, right, targetIndex);
//         }
//     }
//
//     int findKthLargestQuickSelect(std::vector<int> nums, int k) {
//         int n = static_cast<int>(nums.size());
//         // k-й по величине = (n-k)-й по возрастанию (индекс с 0)
//         return quickSelect(nums, 0, n - 1, n - k);
//     }
}

void demo() {

}
}
