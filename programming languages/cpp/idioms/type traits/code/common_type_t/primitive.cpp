#include <iostream>
#include <type_traits>

template<typename T, typename U>
void test();

int main() {
    test<int, double>();
    test<long, unsigned int>();
    test<float, int>();

    return 0;
}

template<typename T, typename U>
void test() {
    std::cout
        << "Common type of "
        << typeid(T).name()
        << " and "
        << typeid(U).name()
        << " is "
        << typeid(
            std::common_type_t<T, U>
        ).name() << std::endl;
}
