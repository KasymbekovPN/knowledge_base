#include "two_sum.h"

#include <unordered_map>
#include <iostream>
#include <format>

namespace two_sum {

std::vector<int> two_sum(const std::vector<int>& numbers, const int target) {
    std::unordered_map<int, int> seen; // value -> index
    seen.reserve(numbers.size());

    for (int i{}; i < static_cast<int>(numbers.size()); ++i) {
        int complement{target - numbers[i]};
        if (auto it = seen.find(complement); it != seen.end()) {
            return {it->second, i};
        }

        seen[numbers[i]] = i;
    }

    return {};
}

void demo() {
    const std::vector<int> NUMBERS = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::cout << "two_sum:";
    for (const std::vector<int> result = two_sum(NUMBERS, 10); int item: result) {
        std::cout << std::format(" {}", item);
    }
    std::cout << std::endl;
}

}