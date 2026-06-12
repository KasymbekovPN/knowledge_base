#include <iostream>
#include <type_traits>

template<typename T, typename R>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int[5], int>();
    test<double[3][4], double>();
    test<char[][2][3], char>();
    test<long, long>();

    return 0;
}

template<typename T, typename R>
void test() {
    std::cout
        << std::is_same_v<std::remove_all_extents_t<T>, R>
        << std::endl;
}
