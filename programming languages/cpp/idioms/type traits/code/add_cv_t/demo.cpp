#include <iostream>
#include <type_traits>

template<typename T, typename C>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int, const volatile int>();
    test<double, const volatile double>();
    
    test<int*, int* const volatile>();
    test<char*, char* const volatile>();

    test<const int, const volatile int>();
    test<const volatile int, const volatile int>();

    return 0;
}

template<typename T, typename C>
void test() {
    std::cout
        << std::is_same_v<std::add_cv_t<T>, C>
        << std::endl;
}
