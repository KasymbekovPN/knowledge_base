#pragma once

#include <string>
#include <vector>

namespace longest_common_subsequence {
    int calc_longest_common_subsequence(const std::string&, const std::string&);
    int calc_longest_common_subsequence_opt(const std::string&, const std::string&);
    std::string reconstruct_lcs(const std::string&, const std::string&, const std::vector<std::vector<int>>&);
    void demo();
}
