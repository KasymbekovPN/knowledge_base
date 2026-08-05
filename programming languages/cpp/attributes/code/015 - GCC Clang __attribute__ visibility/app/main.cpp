#include <mylib/mylib.hpp>

#include <iostream>
#include <format>

int main() {
    const Calculator calc;

    std::cout << std::format("add: {}\n", calc.add(2, 3));
    std::cout << std::format("multiply: {}\n", calc.multiply(2, 3));
    std::cout << std::format("freeFunctionAdd: {}\n", freeFunctionAdd(2, 3));

    // internalOnlyFunction(); // error

    return 0;
}
