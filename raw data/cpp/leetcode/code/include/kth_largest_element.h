#pragma once

#include <vector>

namespace kth_largest_element {
    // solution 1
    int find_kth_largest(const std::vector<int>&, const int);
    // solution 2
    int find_kth_largest_heap(const std::vector<int>&, const int);
    // solution 3
    int find_kth_largest_quick_select(const std::vector<int>&, const int);
    void demo();
}
