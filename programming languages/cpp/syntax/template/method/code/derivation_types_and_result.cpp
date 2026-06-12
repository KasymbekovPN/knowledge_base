#include <iostream>

auto add(const auto& a, const auto& b) {
    return a + b;
}

int main(int argc, char const *argv[]) {
    std::cout << "int\tresult <= " << ::add(1, 3) << std::endl;
    std::cout << "double\tresult <= " << ::add(1.2, 3.4) << std::endl;

    return 0;
}
