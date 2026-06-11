#include <iostream>
#include <cstring>

void _test_strncmp(const std::string&, const std::string&);

int main() {
    _test_strncmp("aaa", "aaa");
    _test_strncmp("aaa", "bbb");
    _test_strncmp("bbb", "aaa");

    return 0;
}

void _test_strncmp(const std::string& str1, const std::string& str2) {
    std::cout
        << "strcmp(" << str1 << ", "  << str2 << ") => "
        << strcmp(str1.c_str(), str2.c_str()) << std::endl;
}
