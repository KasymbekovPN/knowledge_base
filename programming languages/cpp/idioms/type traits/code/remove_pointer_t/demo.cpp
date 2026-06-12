#include <iostream>
#include <type_traits>

template<typename T, typename R>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int*, int>();
    test<const char*, const char>();
    test<double**, double*>();
    test<void*, void>();
    test<int, int>();
    
    return 0;
}

template<typename T, typename R>
void test() {
    std::cout
        << std::is_same_v<std::remove_pointer_t<T>, R>
        << std::endl;
}
