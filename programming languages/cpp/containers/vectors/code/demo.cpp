#include <iostream>
#include <vector>
#include <stdexcept>

void test_vector_by_idx(std::vector<int>*, size_t) noexcept;

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {0, 1, 2, 3, 4};
    std::cout << "first <= " << numbers.front() << std::endl;
    std::cout << "second <= " << numbers[1] << std::endl;
    std::cout << "third <= " << numbers.at(2) << std::endl;
    std::cout << "last <= " << numbers.back() << std::endl;

    numbers[3] = -42;
    for (auto item: numbers) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    size_t last_size = std::size(numbers);
    ::test_vector_by_idx(&numbers, last_size);
    
    numbers.push_back(42);
    ::test_vector_by_idx(&numbers, last_size);

    return 0;
}

void test_vector_by_idx(std::vector<int>* pnumbers, size_t index) noexcept {
    try {
        int result = (*pnumbers).at(index);
        std::cout << "[test] " << result << std::endl;
    } catch(const std::out_of_range& e) {
        std::cerr << e.what() << '\n';
    }
}
