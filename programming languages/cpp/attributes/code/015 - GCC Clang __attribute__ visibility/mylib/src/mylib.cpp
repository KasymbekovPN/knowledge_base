#include "mylib/mylib.hpp"

#include <iostream>

Calculator::Calculator() {
    std::cout << "[mylib] Calculator created\n";
}

int Calculator::add(const int a, const int b) const {
    return a + b;
}
int Calculator::multiply(const int a, const int b) const {
    return internalHelper(a) * b;
}

int Calculator::internalHelper(const int x) const {
    return x;
}

int freeFunctionAdd(const int a, const int b) {
    return a + b;
}

void internalOnlyFunction() {
    std::cout << "[mylib] internal function, not exported from the shared library\n";
}
