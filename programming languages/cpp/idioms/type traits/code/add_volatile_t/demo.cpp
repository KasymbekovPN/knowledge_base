#include <iostream>
#include <type_traits>

template<typename T, typename C>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int, volatile int>();
    test<double, volatile double>();
    test<volatile int, volatile int>();

    test<int*, int* volatile>();
    test<char*, char* volatile>();
    
    return 0;
}

template<typename T, typename C>
void test() {
    std::cout
        << std::is_same_v<std::add_volatile_t<T>, C>
        << std::endl;
}
