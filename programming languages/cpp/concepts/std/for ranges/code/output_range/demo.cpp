#include <iostream>
#include <ranges>
#include <vector>

template<std::ranges::output_range<int> R>
void test(R& _input) {
    for (auto it{_input.begin()}; it != _input.end(); ++it) {
        *it = 42;
    }
}

int main() {
    std::vector<int> vec(5);
    test(vec);
    for (auto&& item: vec) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
