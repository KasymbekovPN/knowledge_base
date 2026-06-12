#include <iostream>
#include <array>

int main() {
    const unsigned SIZE {2};
    std::array<int, SIZE> array {1, 2};

    for (unsigned i {}; i < SIZE; i++) {
        std::cout << "array[" << i << "] <= " << array.at(i) << std::endl;
    }

    try {
        std::cout << array.at(SIZE) << std::endl;
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
