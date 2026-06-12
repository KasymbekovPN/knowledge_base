#include <iostream>
#include <array>

int main() {
    const unsigned SIZE {2};
    std::array<int, SIZE> array {1, 2};

    for (unsigned i {}; i < SIZE; i++) {
        std::cout << "array[" << i << "] <= " << array[i]++ << std::endl;
    }

    for (unsigned i {}; i < SIZE; i++) {
        std::cout << "array[" << i << "] <= " << array[i] << std::endl;
    }

    try {
        array[SIZE] = 42;
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
