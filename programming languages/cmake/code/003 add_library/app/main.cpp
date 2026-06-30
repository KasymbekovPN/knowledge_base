/*
cmake -B .build
cmake --build .build
*/

#include "math.hpp"

#include <iostream>
#include <format>

int main() {
    const int A{42};
    const int B{43};
    std::cout << std::format("add({}, {}) = {}", A, B, add(A, B));
}
