#include <iostream>
#include <string>

int main() {
    std::string str {"a"};
    std::cout << "str <= " << str << std::endl;

    str.push_back('b');
    std::cout << "str <= " << str << std::endl;

    return 0;
}
