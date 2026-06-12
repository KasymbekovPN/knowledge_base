#include <iostream>
#include <ranges>
#include <vector>

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

template<std::ranges::forward_range R>
void test(R&& _range) {
    for (auto& item: _range){
        std::cout << item << " ";
    }
    std::cout << std::endl;

    for (auto& item: _range){
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    test(std::vector({1, 2, 3}));
    test(Range());
    // test(NonRange()); // Error

    return 0;
}
