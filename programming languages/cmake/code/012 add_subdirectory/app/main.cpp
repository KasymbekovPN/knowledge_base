#include "math.hpp"

#include <iostream>
#include <format>

int main() {
    const int A{42};
    std::cout << std::format("square({}) = {}", A, square(A));

    return 0;
}
