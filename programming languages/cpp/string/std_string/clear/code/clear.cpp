#include <iostream>
#include <string>

int main() {
    std::string str {"0123456789"};
    std::cout << "size: " << str.size() << std::endl;

    str.clear();
    std::cout << "size: " << str.size() << std::endl;

    return 0;
}
