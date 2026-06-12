#include <iostream>
#include <vector>
#include <concepts>

struct NonRange {};

struct Range {
    int data[3] = {1, 2, 3};

    int* begin() {
        return data;
    }

    int* end() {
        return data + 3;
    }
};

template<std::ranges::range R>
void test(R&& _input) {
    for (auto&& item: _input) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<int> v {42, 43, 44};
    test(v);

    test(Range());
    // test(NonRange()); // Error

    return 0;
}
