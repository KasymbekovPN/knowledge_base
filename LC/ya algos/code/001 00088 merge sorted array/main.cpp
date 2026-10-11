#include <iostream>
#include <format>
#include <vector>
#include <cstddef>
#include <utility>
#include <string>
#include <queue>

namespace {
    void merge_sorted_array(std::vector<int>& nums1,
                            const int m,
                            std::vector<int>& nums2,
                            const int n) {
        switch((m == 0 ? 2 : 0) + (n == 0 ? 1 : 0)) {
            case 1: {
                nums1.erase(nums1.begin() + m, nums1.end());
                return;
            }
            case 2: { nums1 = nums2; return; }
            case 3: { nums1.clear(); return; }
            default: break;
        }

        std::priority_queue<int, std::vector<int>, std::greater<>> pq(nums2.begin(), nums2.end());

        const std::size_t size{static_cast<std::size_t>(m + n)};
        std::size_t nums1_idx{0};
        for (;nums1_idx < static_cast<std::size_t>(m); ++nums1_idx)
        {
            const auto buffer{pq.top()};
            if (nums1[nums1_idx] <= buffer) continue;

            pq.pop();
            pq.push(nums1[nums1_idx]);
            nums1[nums1_idx] = buffer;
        }

        while (!pq.empty()) {
            nums1[nums1_idx++] = pq.top();
            pq.pop();
        }
    }

    void test_merge_sorted_array(const std::string& label,
                                 std::vector<int>& nums1,
                                 const int m,
                                 std::vector<int>& nums2,
                                 const int n,
                                 const std::vector<int>& expected) {
        merge_sorted_array(nums1, m, nums2, n);
        std::cout << std::format("{}: {}\n",
            label,
            nums1 == expected ? "OK" : "FAIL") << std::flush;
    }
}

int main() {
    std::vector<int> nums1_0{4,5,6,0,0,0};
    std::vector<int> nums2_0{1,2,3};
    std::vector<int> expected_0{1, 2, 3, 4, 5, 6};
    test_merge_sorted_array("t1", nums1_0, 3, nums2_0, 3, expected_0);

    std::vector<int> nums1_1{1, 2, 3, 0, 0, 0};
    std::vector<int> nums2_1{2, 5, 6};
    std::vector<int> expected_1{1, 2, 2, 3, 5, 6};
    test_merge_sorted_array("t1", nums1_1, 3, nums2_1, 3, expected_1);

    std::vector<int> nums1_2{0};
    std::vector<int> nums2_2{1};
    std::vector<int> expected_2{1};
    test_merge_sorted_array("t2", nums1_2, 0, nums2_2, 1, expected_2);

    return 0;
}