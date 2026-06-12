#include <iostream>
#include <type_traits>

template<typename T, typename R>
void test();

int main() {
    std::cout << std::boolalpha;

    test<unsigned int, int>();
    test<unsigned long, long>();
    test<uint8_t, int8_t>();
    test<uint16_t, int16_t>();
    test<uint32_t, int32_t>();
    test<uint64_t, int64_t>();

    return 0;
}

template<typename T, typename R>
void test() {
    std::cout
        << std::is_same_v<std::make_signed_t<T>, R>
        << std::endl;
}
