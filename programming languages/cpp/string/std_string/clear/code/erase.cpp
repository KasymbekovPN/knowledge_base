#include <iostream>
#include <string>

int main() {
    std::string str {"0123456789"};
    std::cout << "str <= " << str << std::endl;

    str.erase(2, 5);
    std::cout << "str <= " << str << std::endl;

    return 0;
}
