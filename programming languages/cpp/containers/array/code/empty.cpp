#include <iostream>
#include <array>

template<typename T, int N>
void print(const std::array<T,N>& array) {
    std::cout << "Is empty: " << std::boolalpha
              << array.empty() << std::noboolalpha
              << std::endl;
}

int main() {
    std::array<int, 0> empty_numbers {};
    print<int, 0>(empty_numbers);

    std::array<int, 5> numbers {1, 2, 3, 4, 5};
    print<int, 5>(numbers);

    return 0;
}
