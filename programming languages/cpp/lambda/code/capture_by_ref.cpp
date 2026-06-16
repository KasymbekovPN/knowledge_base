#include <iostream>

int main() {
    int x {42};
    
    auto f = [&]() {
        x += 1;
        std::cout << "Inner: " << x << std::endl;
    };
    f();
    
    std::cout << "Outer: " << x << std::endl;

    return 0;
}
