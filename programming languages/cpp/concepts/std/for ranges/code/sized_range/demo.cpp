#include <iostream>
#include <vector>
#include <ranges>

struct Range {
    int data[3]{1, 2, 3};

    int* begin() {
        return data;
    }

    int* end() {
        return data + 3;
    }

    std::size_t size() const {
        return 3;
    }
};

template<std::ranges::sized_range T>
void test(const T& _input) { 
    std::cout << "Size: " << std::ranges::size(_input) << std::endl;
}

int main() {
    const auto&& vec = std::vector<int>({1, 2, 3, 4, 5});
    test(vec);

    const auto&& range = Range();
    test(range);

    return 0;
}
