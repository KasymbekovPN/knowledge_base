#include <iostream>
#include <vector>

int main() {
    std::vector<int> numbers {1, 2, 3};
    std::cout << "numbers size: " << numbers.size() << std::endl;

    numbers.clear();
    std::cout << "numbers size: " << numbers.size() << std::endl;

    return 0;
}
