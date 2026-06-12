#include <iostream>
#include <ranges>
#include <vector>
#include <concepts>
#include <span>

struct Range {
    int arr[3]{1, 2, 3};

    int* begin() {
        return arr;
    }

    int* end() {
        return arr + 3;
    }
};

template<>
inline constexpr bool
    std::ranges::enable_borrowed_range<Range> = true;

template<std::ranges::borrowed_range T>
void test(T& range) {
    for (auto& i: range) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

int main() {
    int arr[]{100, 101, 102};
    auto&& s = std::span<int>(arr);
    test(s);

    auto&& range = Range();
    test(range);

    auto&& vec = std::vector({42, 43});
    // test(vec); // Error

    return 0;
}
