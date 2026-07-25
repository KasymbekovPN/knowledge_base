#include "trapping_rain_water.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <format>
#include <concepts>
#include <ostream>

namespace trapping_rain_water {

#define PRINT

template <typename T>
concept Streamable = requires(std::ostream& os, const T& value) {
    { os << value } -> std::same_as<std::ostream&>;
};

template <Streamable T>
static void print(const std::vector<T>& vec, const std::string& label) {
#ifdef PRINT
    std::cout << std::format("{}: ", label);
    for (const auto& elem : vec) {
        std::cout << elem << ' ';
    }
    std::cout << '\n';
#endif
}

int trap(const std::vector<int>& heights) {
    const int N{static_cast<int>(heights.size())};
    if (N == 0) return 0;

    std::vector<int> max_left(N), max_right(N);

    max_left[0] = heights[0];
    for (int i{1}; i < N; ++i) {
        max_left[i] = std::max(max_left[i - 1], heights[i]);
    }

    max_right[N - 1] = heights[N - 1];
    for (int i{N-2}; i >= 0; --i) {
        max_right[i] = std::max(max_right[i + 1], heights[i]);
    }

    int total{};
    for (int i{}; i < N; ++i) {
        total += std::min(max_left[i], max_right[i]) - heights[i];
    }

    print(heights, "heights");
    print(max_left, "max_left");
    print(max_right, "max_right");

    return total;
}

int trap_two_pointer(const std::vector<int>& heights) {
    const int N{static_cast<int>(heights.size())};
    if (N == 0) return 0;

    int left{}, right{N-1};
    int left_max{}, right_max{};
    int total{};

    while (left < right) {
        if (heights[left] < heights[right]) {
            left_max = std::max(left_max, heights[left]);
            total += left_max - heights[left];
            ++left;
        } else {
            right_max = std::max(right_max, heights[right]);
            total += right_max - heights[right];
            --right;
        }
    }

    return total;
}

void demo() {
    const std::vector<int> HEIGHTS{0,1,0,2,1,0,1,3,2,1,2,1};

    std::cout << std::format("TRAP {}\n", trap(HEIGHTS));
    std::cout << std::format("TRAP 2P {}\n", trap_two_pointer(HEIGHTS));
}
}