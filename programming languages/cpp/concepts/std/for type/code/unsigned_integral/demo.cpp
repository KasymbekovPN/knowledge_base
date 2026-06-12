#include <iostream>
#include <concepts>

template<std::unsigned_integral T>
void test(T);

int main(int argc, char const *argv[]) {
    test(42u);
    test(1000UL);
    test(size_t{123});
    test(uint32_t{999});

    // test(-5); // Error
    // test(3.14159); // Error

    return 0;
}

template<std::unsigned_integral T>
void test(T _input) {
    std::cout << "Input: " << _input << std::endl;
}
