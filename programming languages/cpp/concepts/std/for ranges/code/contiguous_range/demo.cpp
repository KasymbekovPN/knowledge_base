#include <iostream>
#include <vector>
#include <ranges>

struct NonRange {};

struct Range {
    static const size_t SIZE{5};
    int data[SIZE] {1, 2, 3, 4, 5};

    int* begin() {
        return data;
    }

    int* end() {
        return data + SIZE;
    }
};

template<std::ranges::random_access_range R>
void test(R&& _range) {
    std::cout << std::ranges::data(_range)[0] << std::endl;
}

int main() {
    test(std::vector<int>({11, 21, 31}));
    test(Range());
    // test(NonRange()); // Error

    return 0;
}
