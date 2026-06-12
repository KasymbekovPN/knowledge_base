#include <iostream>
#include <concepts>

template<typename T>
requires std::convertible_to<T, double>
void test_number(T);

template<typename T>
requires std::convertible_to<T, std::string>
void test_string(T);

int main() {
    test_number(42);
    test_number(2.72f);
    test_string("hi");
    test_string(std::string("hello"));
    
    return 0;
}

template<typename T>
requires std::convertible_to<T, double>
void test_number(T _input) {
    std::cout
        << "Numberic: "
        << static_cast<double>(_input)
        << std::endl;
}

template<typename T>
requires std::convertible_to<T, std::string>
void test_string(T _input) {
    std::cout
        << "String: "
        << std::string(_input)
        << std::endl;
}
