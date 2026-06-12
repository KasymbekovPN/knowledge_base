#include <iostream>
#include <type_traits>

template<typename T, typename R>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int[5], int>();
    test<double[10], double>();
    test<char[2][2], char[2]>();
    test<int, int>();

    return 0;
}

template<typename T, typename R>
void test() {
    std::cout
        << std::is_same_v<std::remove_extent_t<T>, R>
        << std::endl;
}
