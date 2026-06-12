#include <iostream>
#include <vector>
#include <algorithm>

void print_vector(const std::vector<int>&);

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {1, 2, 3, 4, 5, 3, 7};
    print_vector(numbers);

    std::vector<int>::iterator it = std::remove(numbers.begin(), numbers.end(), 3);
    numbers.erase(it, numbers.end());
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
