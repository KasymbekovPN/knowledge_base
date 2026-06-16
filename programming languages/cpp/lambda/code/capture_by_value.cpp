#include <iostream>

int main() {
    int x {42};
    
    auto f = [=]() {
        // x += 1; //  error: cannot assign to a variable captured by copy in a non-mutable lambda
        std::cout << "Inner 0: " << x << std::endl;
    };
    f();

    auto m = [=]() mutable {
        x += 1;
        std::cout << "Inner 1: " << x << std::endl;
    };
    m();

    std::cout << "Outer: " << x << std::endl;

    return 0;
}
