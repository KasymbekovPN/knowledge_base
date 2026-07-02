#include "calc/calc.hpp"
#include <stdexcept>

int add(const int a, const int b) { return a + b; }

int divide(const int a, const int b) {
    if (b == 0) throw std::invalid_argument("Division by zero");
    return a / b;
}
