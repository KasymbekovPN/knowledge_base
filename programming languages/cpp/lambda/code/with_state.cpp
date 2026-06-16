#include <iostream>

int main() {
    const int MULTIPLIER {5};

    auto multiply = [MULTIPLIER](int x) -> int {return x * MULTIPLIER;};
    std::cout << "mul => " << multiply(12) << std::endl;

    auto counter = [x = 0]() mutable {return ++x;};
    std::cout << counter() << std::endl;
    std::cout << counter() << std::endl;
    std::cout << counter() << std::endl;

    return 0;
}
