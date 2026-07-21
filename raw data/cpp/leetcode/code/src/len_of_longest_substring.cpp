#include "len_of_longest_substring.h"

#include <iostream>
#include <format>
#include <unordered_map>
#include <algorithm>

namespace len_of_longest_substring {

int get_longest_substring(const std::string &line) {
    std::unordered_map<char, int> last_seen;
    int best{};
    int left{};

    for (int right{}; right < static_cast<int>(line.length()); ++right) {
        char c{line[right]};
        if (auto it = last_seen.find(c); it != last_seen.end() && it->second >= left) {
            left = it->second + 1; // сдвигаем левую границу за дубликат
        }

        last_seen[c] = right;
        best = std::max(best, right - left + 1);
    }

    return best;
}

void demo() {
    const std::string line{"0123011234566789"};
    std::cout << std::format("best: {}\n", get_longest_substring(line));
}

}