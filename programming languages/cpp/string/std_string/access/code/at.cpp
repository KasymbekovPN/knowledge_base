#include <iostream>
#include <string>

void _test_c_str(const std::string&, size_t);

int main() {
    const std::string str {"Hello"};
    for (size_t i {}; i < str.size(); i++) {
        _test_c_str(str, i);
    }

    return 0;
}

void _test_c_str(const std::string& str, size_t idx) {
    try {
        std::cout << str.at(idx) << std::endl;
    } catch(const std::out_of_range& e) {
        std::cerr << e.what() << std::endl;
    }
}
