#include <iostream>
#include <vector>
#include <ranges>

struct NonRange {};

struct Range {
    int data[5] {1, 2, 3, 4, 5};

    int* begin() {
        return data;
    }

    int* end() {
        return data + 5;
    }
};

template<std::ranges::bidirectional_range R>
void test(R&& _range) {
    for (auto& item: _range){
        std::cout << item << " ";
    }
    std::cout << std::endl;

    auto&& it = std::ranges::end(_range);
    while (it != std::ranges::begin(_range)) {
        --it;
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

int main() {
    test(std::vector<int>({1, 2, 3}));
    test(Range());
    // test(NonRange()); // Error

    return 0;
}
