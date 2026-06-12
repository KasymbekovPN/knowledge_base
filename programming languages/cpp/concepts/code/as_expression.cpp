#include <iostream>
#include <concepts>

template<typename T>
T test(T, T)
requires (std::integral<T> || std::floating_point<T>);

int main(int argc, char const *argv[]) {
    std::cout << (test(36, 12)) << std::endl;
    std::cout << test(3.0f, 12.0f) << std::endl;
    // std::cout << test("3.0f", "12.0f") << std::endl;

    return 0;
}

template<typename T>
T test(T a, T b)
requires (std::integral<T> || std::floating_point<T>) {
    return b != 0 ? a / b : 0;
}
