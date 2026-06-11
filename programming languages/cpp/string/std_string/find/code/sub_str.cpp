#include <iostream>
#include <string>

void _test_sub_str(const std::string&, const size_t, const size_t);

int main() {
    const std::string str {"Hello"};
    for (size_t i {}; i < 5; i++) {
        _test_sub_str(str, 3, i);
    }

    return 0;
}

void _test_sub_str(const std::string& str, const size_t pos, const size_t len) {
    std::cout
        << "[" << pos << ", " << len << "] "
        << str.substr(pos, len) << std::endl;
}
