#include <iostream>
#include <type_traits>

template<typename T, typename C>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int, const int>();
    test<const int, const int>();
    test<double, const double>();

    test<int*, int* const>();
    test<char*, char* const>();

    test<const char*, const char*>();
    
    return 0;
}

template<typename T, typename C>
void test() {
    std::cout
        << std::is_same_v<std::add_const_t<T>, C>
        << std::endl;
}
