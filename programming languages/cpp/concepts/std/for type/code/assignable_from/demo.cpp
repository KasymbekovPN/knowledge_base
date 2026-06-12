#include <iostream>
#include <concepts>

template<typename T, typename U>
requires std::assignable_from<T&, const U&>
void test(T&, const U&);

int main() {
    int x{0};
    double d{3.14};
    test(x, d);
    test(d, 5);

    const int y {y};
    // test(y, 5); // Error

    return 0;
}

template<typename T, typename U>
requires std::assignable_from<T, U>
void test(T& _target, const U& _input) {
    _target = _input;
    std::cout << _input << " :: " << _target << std::endl;
}
