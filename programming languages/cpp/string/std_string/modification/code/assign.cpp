#include <iostream>
#include <string>

int main() {
    std::string str {"hello"};
    std::cout << "str <= " << str << std::endl;

    std::cout << "str <= " << str.assign("world") << std::endl;
    std::cout << "str <= " << str << std::endl;

    return 0;
}
