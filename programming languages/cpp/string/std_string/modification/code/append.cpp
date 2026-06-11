#include <iostream>
#include <string>

int main() {
    std::string str {"aaa"};
    str += "bbb";
    str.append("ccc");

    std::cout << "str <= " << str << std::endl;

    return 0;
}
