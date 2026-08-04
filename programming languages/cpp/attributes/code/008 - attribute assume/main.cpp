#include <iostream>
#include <format>

namespace {
    int divide(const int x, const int y) {
        [[assume(y != 0)]];
        return x / y;
    }

    void process(const int n) {
        [[assume(n > 0 && n <= 100)]];
        for (int i{}; i < n; ++i) {
            std::cout << std::format("{}\n", i);
        }
    }
}

int main() {
    std::cout << std::format("divide: {}\n", divide(10, 5));
    process(3);
}

