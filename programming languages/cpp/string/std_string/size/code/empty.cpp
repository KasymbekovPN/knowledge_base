#include <iostream>
#include <string>

void _test_empty(const std::string&);

int main() {
    const std::string empty {};
    const std::string not_empty {"hello"};

    _test_empty(empty);
    _test_empty(not_empty);

    return 0;
}

void _test_empty(const std::string& str) {
    std::cout
        << "Is it empty => "
        << std::boolalpha
        << str.empty()
        << std::noboolalpha
        << std::endl;
}
