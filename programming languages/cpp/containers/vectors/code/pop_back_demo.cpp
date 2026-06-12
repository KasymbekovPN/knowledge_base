#include <iostream>
#include <vector>

void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3};
    print_vector(numbers);

    numbers.pop_back();
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
