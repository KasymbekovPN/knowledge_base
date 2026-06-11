#include <iostream>
#include <cstring>

void _test_strncmp(const std::string&, const std::string&, size_t);

int main() {
    _test_strncmp("aaa", "aaa", 3);
    _test_strncmp("aaa0", "aaa2", 3);
    _test_strncmp("aaa", "bbb", 3);
    _test_strncmp("bbb", "aaa", 3);

    return 0;
}

void _test_strncmp(const std::string& str1, const std::string& str2, size_t len) {
    std::cout
        << "strcmp(" << str1 << ", "  << str2 << ") => "
        << strncmp(str1.c_str(), str2.c_str(), len) << std::endl;
}
