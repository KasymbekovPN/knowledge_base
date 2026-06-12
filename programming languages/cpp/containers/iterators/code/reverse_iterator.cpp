#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {1, 2, 3};
    for (auto it {numbers.rbegin()}; it != numbers.rend(); it++) {
        std::cout << *it << "\t";
    }
    std::cout << std::endl;

    for (auto it {numbers.crbegin()}; it != numbers.crend(); it++) {
        // *it = (*it) + 1; //< Error
        std::cout << *it << "\t";
    }

    return 0;
}
