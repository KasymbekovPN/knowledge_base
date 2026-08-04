#include <format>
#include <iostream>

namespace {

    __attribute__((const))
    int square(const int x) {
        return x * x;
    }

    __attribute__((const))
    int add(const int a, const int b) {
        return a + b;
    }

}

int main() {
    std::cout << std::format("square: {}\n", square(42));
    std::cout << std::format("add: {}\n", add(42, 42));
    std::cout << std::format("add: {}\n", add(1, 1));

    return 0;
}
