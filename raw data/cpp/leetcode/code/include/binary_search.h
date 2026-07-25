#pragma once

#include <vector>

namespace binary_search {
    int binary_search(const std::vector<int>&, const int);
    int binary_search_recursive(const std::vector<int>&, const int, int, int);
    int binary_search_stl(const std::vector<int>&, const int);
    void demo();
}
