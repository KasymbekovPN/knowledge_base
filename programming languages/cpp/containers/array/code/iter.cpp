#include <iostream>
#include <array>

int main() {
    std::array<int, 5> numbers {1, 2, 3, 4, 5};

    for (auto number: numbers){
        std::cout << number << " ";
    }
    std::cout << std::endl;

    for (auto &&number: numbers){
        std::cout << number << " ";
    }
    std::cout << std::endl;

    for (std::array<int, 5>::iterator it {numbers.begin()}; it != numbers.end(); it++){
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    return 0;
}
