#include <iostream>
#include <concepts>

template<std::floating_point T>
void test(T);

int main(int argc, char const *argv[]) {
    test(3.14f);
    test(2.718);
    test(1.414L);

    // test(42); // Error
    // test("hello"); // Error

    return 0;
}

template<std::floating_point T>
void test(T _input) {
    std::cout << "Input: " << _input << std::endl;
}
