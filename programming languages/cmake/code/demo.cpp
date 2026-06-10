#pragma once
#include <iostream>

#include "value.hpp"

int main() {
    auto&& v = custom_ns::Value(42);
    std::cout << v << std::endl;
    
    return 0;
}
