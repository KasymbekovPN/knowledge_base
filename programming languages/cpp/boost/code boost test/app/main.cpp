
import calculator;

#include <iostream>
#include <format>

int main() {
    const int A{2};
    const int B{3};
    const int C{42};
    const int D{6};

    std::cout << std::format("{} + {} = {}", A, B, calc::Calculator::add(A, B));
    std::cout << std::format("{} / {} = {}", C, D, calc::Calculator::divide(A, B));

    return 0;
}
