#include <iostream>
#include <type_traits>

enum Color { Red };

struct Widget {};

template<typename T>
void test();

int main() {
    test<int>();
    test<double>();
    test<bool>();
    test<char*>();
    test<const int*>();
    test<void*>();
    test<std::nullptr_t>();
    test<Color>();
    test<Widget>();
    test<int[4]>();
    test<void>();
    test<int&>();
    test<void()>();

    return 0;
}

template<typename T>
void test() {
    if constexpr (std::is_scalar_v<T>) {
        std::cout << "A scalar object";
    } else {
        std::cout << "Not a scalar object";
    }
    std::cout << std::endl;
}
