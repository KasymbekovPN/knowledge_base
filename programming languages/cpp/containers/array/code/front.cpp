#include <iostream>
#include <array>

template<typename T, int N>
void print_array(const std::array<T,N>& array) {
    for (auto &&item: array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::array<int, 5> numbers {1, 2, 3, 4, 5};
    print_array<int, 5>(numbers);

    int first = numbers.front();
    std::cout << "First: " << first << std::endl;
    print_array<int, 5>(numbers);

    return 0;
}
