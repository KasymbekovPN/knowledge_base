#include <iostream>
#include <array>

int main() {
    std::array<int, 3> numbers0 {1, 2, 3};
    std::array<int, 3> numbers1 {1, 2, 4};

    if (numbers0 == numbers1) {
        std::cout << "numbers0 == numbers1" << std::endl;
    } else {
        std::cout << "numbers0 != numbers1" << std::endl;
    }

    if (numbers0 < numbers1) {
        std::cout << "numbers0 < numbers1" << std::endl;
    } else {
        std::cout << "numbers0 >= numbers1" << std::endl;
    }

    return 0;
}
