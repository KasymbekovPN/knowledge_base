#include <iostream>
#include <vector>

void print_vector(const std::string&, const std::vector<int>&);

int main() {
    std::vector<int> numbers0 {0, 1, 2, 3, 4};
    std::vector<int> numbers1 {5, 6, 7, 8, 9};
    
    std::cout << "original:" << std::endl;
    print_vector("numbers0", numbers0);
    print_vector("numbers1", numbers1);

    std::cout << "after swap:" << std::endl;
    numbers0.swap(numbers1);
    print_vector("numbers0", numbers0);
    print_vector("numbers1", numbers1);

    std::cout << "after swap with empty vector:" << std::endl;
    std::vector<int>().swap(numbers0);
    print_vector("numbers0", numbers0);

    return 0;
}

void print_vector(const std::string& label, const std::vector<int>& numbers) {
    std::cout << "[" << label << "] size: " << numbers.size()
        << ", capacity: " << numbers.capacity()
        << " :: ";
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
