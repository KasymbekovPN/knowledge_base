#include <iostream>
#include <string>

void _test_find_last_not_of(const std::string&, const std::string);

int main() {
    const std::string str {"hello"};
    _test_find_last_not_of(str, "abc");
    _test_find_last_not_of(str, "el");
    _test_find_last_not_of(str, "l");

    return 0;
}

void _test_find_last_not_of(const std::string& str, const std::string sub) {
    auto idx = str.find_first_of(sub);
    if (idx != std::string::npos) {
        std::cout << "idx <= " << idx << std::endl;
    } else {
        std::cout << "NPOS" << std::endl;
    }
}
