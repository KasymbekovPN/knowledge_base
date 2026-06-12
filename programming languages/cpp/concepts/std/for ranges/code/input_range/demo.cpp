#include <iostream>
#include <ranges>
#include <vector>

struct NonRange {};

struct Range {
    int data[3]{1, 2, 3};

    int* begin() {
        return data;
    }

    int *end() {
        return data + 3;
    }
};

template<std::ranges::input_range R>
void test(R&& _range) {
    for (auto&& item: _range) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    test(std::vector<int>({11, 12, 13}));
    test(Range());
    // test(NonRange()); // Error

    return 0;
}
