#include <iostream>
#include <type_traits>

template<typename T>
typename std::enable_if<std::is_integral_v<T>, void>::type
process(T&& _value) {
    std::cout
        << "Integer: "
        << _value
        << std::endl;
}

template<typename T>
typename std::enable_if<std::is_floating_point_v<T>, void>::type
process(T&& _value) {
    std::cout
        << "Float: "
        << _value
        << std::endl;
}

int main() {
    process(42);
    process(12.34);

    return 0;
}
