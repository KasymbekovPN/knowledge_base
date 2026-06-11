#include <iostream>
#include <string>

void _test_length(const std::string&);

int main() {
    const std::string empty {};
    const std::string not_empty {"hello"};

    _test_length(empty);
    _test_length(not_empty);

    return 0;
}

void _test_length(const std::string& str) {
    std::cout
        << "Length => "
        << str.length()
        << std::endl;
}
