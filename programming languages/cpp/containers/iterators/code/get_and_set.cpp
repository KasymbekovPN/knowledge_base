#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {1, 2, 3};
    
    auto it {numbers.begin()};
    std::cout << "numbers[0] <= " << *it << std::endl;

    *it = 42;
    std::cout << "numbers[0] <= " << *it << std::endl;

    it += 2;
    std::cout << "numbers[2] <= " << *it << std::endl;

    return 0;
}
