#include <iostream>

template<typename T> T add(T, T);

int main(int argc, char const *argv[]) {
    std::cout << "int\t: " << add(42, 43) << std::endl;
    std::cout << "double\t: " << add(12.3, 45.6) << std::endl;
    std::cout
        << "string\t: "
        << add(std::string("hello, "), std::string("world!!!"))
        << std::endl;

    return 0;
}

template<typename T> T add(T a, T b) {
    return a + b;
}
