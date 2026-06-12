#include <iostream>
#include <concepts>

template<std::signed_integral T>
void test(T);

int main() {
    test(42);
    test('A');
    // test(true); // Error
    // test(100U); // Error
    // test(std::string("hello")); // Error    

    return 0;
}

template<std::signed_integral T>
void test(T _input) {
    std::cout << "Value: " << _input << std::endl;
}
