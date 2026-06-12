#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {1, 2, 3};
    
    auto it {numbers.begin()};
    while (it != numbers.end()) {
        std::cout << "[while] value <= " << *it << std::endl;
        ++it;
    }
    
    for (auto it {numbers.begin()}; it != numbers.end(); it++) {
        std::cout << "[for] value <= " << *it << std::endl;
    }

    return 0;
}
