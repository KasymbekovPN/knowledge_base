#include <iostream>

int main() {
    auto sum = [](int a, int b) -> int {
        return a + b;
    };
    std::cout << sum(42, 1) << std::endl;

    return 0;
}
