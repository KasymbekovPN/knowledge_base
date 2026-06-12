#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    int data[] {100, 101, 102, 103};
    for (auto it {std::begin(data)}; it != std::end(data); it++) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    std::vector<int> numbers {1, 2, 3};
    numbers.insert(numbers.end(), std::begin(data) + 1, std::end(data) - 1);
    for (auto it {numbers.begin()}; it != numbers.end(); it++) {
        std::cout << *it << " ";
    }

    return 0;
}
