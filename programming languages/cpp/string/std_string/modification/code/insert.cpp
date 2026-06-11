#include <iostream>
#include <string>

int main() {
    std::string str {"____"};
    str.insert(2, "hello");
    std::cout << "str <= "<< str << std::endl;

    return 0;
}
