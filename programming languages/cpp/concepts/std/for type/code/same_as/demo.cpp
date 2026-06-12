#include <iostream>
#include <concepts>

template<typename T, typename U>
void test();

template<typename T>
requires std::same_as<T, int>
void print(T);

template<typename T>
requires std::same_as<T, double>
void print(T);

int main() {
    test<int, int>();
    test<int, const int>();
    test<int, int&>();
    test<int*, int*>();
    test<int, double>();

    print(42);
    print(12.34);
    // print(333.0f); // Error
    
    return 0;
}

template<typename T, typename U>
void test() {
    if constexpr (std::same_as<T, U>) {
        std::cout << "Same";
    } else {
        std::cout << "Different";
    }
    std::cout << std::endl;
}

template<typename T>
requires std::same_as<T, int>
void print(T _input) {
    std::cout << "int: " << _input << std::endl;
}

template<typename T>
requires std::same_as<T, double>
void print(T _input) {
    std::cout << "double: " << _input << std::endl;
}
