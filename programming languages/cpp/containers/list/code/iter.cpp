#include <iostream>
#include <list>

int main(int argc, char const *argv[]) {
    std::list<int> numbers {1, 2, 3};

    for (auto &&number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;

    for (auto &number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;

    for (const auto &number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;

    for (auto it {numbers.begin()}; it != numbers.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    return 0;
}
