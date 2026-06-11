#include <iostream>
#include <string>

void _test_find(const std::string&, const std::string);

int main() {
    const std::string str {"hello"};
    _test_find(str, "l");
    _test_find(str, "x");

    return 0;
}

void _test_find(const std::string& str, const std::string sub) {
    auto idx = str.find(sub);
    if (idx != std::string::npos) {
        std::cout << "idx <= " << idx << std::endl;
    } else {
        std::cout << "NPOS" << std::endl;
    }
}
