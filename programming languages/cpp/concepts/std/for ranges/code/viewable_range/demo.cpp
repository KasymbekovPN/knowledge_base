#include <iostream>
#include <vector>
#include <ranges>

template<std::ranges::viewable_range R>
void test(R&& _input) {
    auto v = std::views::all(_input);
    for (auto item: v) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    test(std::vector<int>({1, 2, 3}));

    return 0;
}
