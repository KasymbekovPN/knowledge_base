#include <iostream>
#include <string>

void _test_capacity(const std::string&);

int main() {
    std::string str {"Hello"};
    _test_capacity(str);

    str.reserve(32);
    _test_capacity(str);

    str.shrink_to_fit();
    _test_capacity(str);

    return 0;
}

void _test_capacity(const std::string& str) {
    std::cout
        << "capacity <= " << str.capacity()
        << " value <= " << str
        << std::endl;
}
