#include <iostream>
#include <string>

void _test_size(const std::string&);

int main() {
    const std::string empty {};
    const std::string not_empty {"hello"};

    _test_size(empty);
    _test_size(not_empty);

    return 0;
}

void _test_size(const std::string& str) {
    std::cout
        << "Size => "
        << str.size()
        << std::endl;
}
