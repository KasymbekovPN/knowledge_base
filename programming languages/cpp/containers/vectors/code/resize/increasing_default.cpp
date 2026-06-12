#include <iostream>
#include <vector>


void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3, 4, 5};
    print_vector(numbers);

    numbers.resize(7, 123);
    print_vector(numbers);

    return 0;
}


void print_vector(const std::vector<int>& numbers) {
    std::cout << "size: " << numbers.size()
        << ", capacity: " << numbers.capacity()
        << " :: ";
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
