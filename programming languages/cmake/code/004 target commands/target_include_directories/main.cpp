/*
cmake -B .build
cmake --build .build
*/
#include "geometry/circle.hpp"

#include <iostream>
#include <format>

#include "include/geometry/circle.hpp"

int main() {
    constexpr double RADIUS{2.0};
    std::cout << std::format("circle_area({}) = {}", RADIUS, circle_area(RADIUS));
}
