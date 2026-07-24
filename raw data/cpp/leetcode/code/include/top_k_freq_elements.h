#pragma once

#include <vector>

namespace top_k_freq_elements {
    std::vector<int> top_k_freq(const std::vector<int>&, const int);
    std::vector<int> top_k_freq_bucket(const std::vector<int>&, const int);
    void demo();
}