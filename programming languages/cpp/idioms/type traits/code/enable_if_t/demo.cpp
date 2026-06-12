#include <iostream>
#include <type_traits>

template<typename T>
std::enable_if_t<std::is_integral_v<T>, int>
test(T);

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, float>
test(T);

int main(int argc, char const *argv[]) {
    std::cout
        << "int output"
        << test(12)
        << std::endl;

    std::cout
        << "float output"
        << test(4.2f)
        << std::endl;

    // std::cout
    //     << "string output"
    //     << test("hello")
    //     << std::endl;

    return 0;
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>, int>
test(T _input) {
    std::cout << "int input: " << _input << std::endl;
    return _input * _input;
}

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, float>
test(T _input) {
    std::cout << "float input: " << _input << std::endl;
    return _input * _input;
}
