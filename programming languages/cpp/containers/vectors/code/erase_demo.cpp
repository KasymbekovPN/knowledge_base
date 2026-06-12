#include <iostream>
#include <vector>

void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3, 4, 5, 6, 7};
    print_vector(numbers);

    numbers.erase(numbers.begin() + 1);
    print_vector(numbers);

    numbers.erase(numbers.begin() + 2, numbers.begin() + 4);
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
