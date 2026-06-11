#include <iostream>
#include <string>

void _test_pop_back(std::string&);

int main() {
    std::string str {"01"};

    const size_t SIZE = str.size();
    for (size_t i {}; i <= SIZE; i++) {
        _test_pop_back(str);
    }

    return 0;
}

void _test_pop_back(std::string& str) {
    if (str.empty()) {
        std::cout << "It's empty" << std::endl;
    } else {
        std::cout << str <<  std::endl;
        str.pop_back();
    }
}
