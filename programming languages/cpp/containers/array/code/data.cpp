#include <iostream>
#include <array>

int main() {
    std::array<int, 3> numbers {1, 2, 3};

    int* ptr = numbers.data();
    for (size_t i = 0; i < numbers.size(); i++) {
        std::cout << ptr[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}
