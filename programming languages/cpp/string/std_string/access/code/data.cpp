#include <iostream>
#include <string>

void _test(const std::string&);

int main() {
    std::string str {"Hello World"};
    std::string nt_str = {"abc\0def"};

    _test(str);
    _test(nt_str);

    return 0;
}

void _test(const std::string& str) {
    std::cout << "#####" << std::endl;
    std::cout << "str <= " << str << std::endl;
    std::cout << "size <= " << str.size() << std::endl;

    const char* ptr = str.data();
    for (size_t i {}; i < str.size(); i++) {
        if (ptr[i] == '\0') {
            std::cout << "\\0";
        } else {
            std::cout << ptr[i];
        }
    }
    std::cout << std::endl;
}
