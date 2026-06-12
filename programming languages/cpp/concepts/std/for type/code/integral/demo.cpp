#include <iostream>
#include <concepts>

template<std::integral T>
void test(T);

int main() {
    test(42);
    test('A');
    test(true);
    test(100LL);
    // test(std::string("hello")); // Error    

    return 0;
}

template<std::integral T>
void test(T _input) {
    std::cout << "Value: " << _input << std::endl;
}
