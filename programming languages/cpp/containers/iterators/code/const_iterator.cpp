#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    const std::vector<int> NUMBERS {1, 2, 3};
    for (std::vector<int>::const_iterator it {NUMBERS.cbegin()}; it != NUMBERS.cend(); it++) {
        std::cout << "[#0] <= " << *it << std::endl;
        // *it = (*it) * (*it); // <= Error (0)
    }
    
    std::vector<int> numbers {1, 2, 3};
    for (std::vector<int>::const_iterator it {numbers.cbegin()}; it != numbers.cend(); it++) {
        std::cout << "[#1] <= " << *it << std::endl;
        // *it = (*it) * (*it); // <= Error (1)
    }

    return 0;
}
