#include "binary_search.h"

#include <algorithm>
#include <vector>
#include <iostream>
#include <format>

namespace binary_search {

static constexpr int BAD_RESULT{-1};

int binary_search(const std::vector<int>& sorted_container, const int target) {
    int left{0};
    int right{static_cast<int>(sorted_container.size() - 1)};

    while (left <= right) {
        // защита от переполнения
        const int mid{left  + (right - left) / 2 };

        if (sorted_container[mid] == target) {
            return mid;
        }

        if (sorted_container[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return BAD_RESULT;
}

int binary_search_recursive(const std::vector<int>& sorted_container,
                            const int target,
                            const int left,
                            const int right) {
    if (left > right) return BAD_RESULT;

    const int mid{left  + (right - left) / 2 };

    if (sorted_container[mid] == target) return mid;

    return
        sorted_container[mid] < target
        ? binary_search_recursive(sorted_container, target, mid + 1, right)
        : binary_search_recursive(sorted_container, target, left, mid - 1);
}

int binary_search_stl(const std::vector<int>& sorted_container, const int target) {
    if (const auto it = std::ranges::lower_bound(sorted_container, target);
        it != sorted_container.end() && *it == target) {
        return static_cast<int>(it - sorted_container.begin());
    }

    return BAD_RESULT;
}

void demo() {
    constexpr int BAD_TARGET{2};
    constexpr int GOOD_TARGET{9};
    const std::vector<int> SORTED_COINTAINER{-1, 0, 3, 5, 9, 12};

    std::cout << std::format("target: {} => {}\n", BAD_TARGET, binary_search(SORTED_COINTAINER, BAD_TARGET));
    std::cout << std::format("target: {} => {}\n", GOOD_TARGET, binary_search(SORTED_COINTAINER, GOOD_TARGET));

    std::cout << std::format("target rec: {} => {}\n", BAD_TARGET, binary_search_recursive(
        SORTED_COINTAINER,
        BAD_TARGET,
        0,
        static_cast<int>(SORTED_COINTAINER.size()) - 1));
    std::cout << std::format("target rec: {} => {}\n", GOOD_TARGET, binary_search_recursive(
        SORTED_COINTAINER,
        GOOD_TARGET,
        0,
        static_cast<int>(SORTED_COINTAINER.size()) - 1));

    std::cout << std::format("target stl: {} => {}\n", BAD_TARGET, binary_search_stl(
        SORTED_COINTAINER,
        BAD_TARGET));
    std::cout << std::format("target stl: {} => {}\n", GOOD_TARGET, binary_search_stl(
        SORTED_COINTAINER,
        GOOD_TARGET));
}

}