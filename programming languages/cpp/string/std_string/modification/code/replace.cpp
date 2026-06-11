#include <iostream>
#include <string>

int main() {
    std::string str {"__hello__"};
    std::cout << "str <= "<< str << std::endl;

    std::cout << "str <= "<< str.replace(2, 5, "world") << std::endl;
    std::cout << "str <= "<< str << std::endl;

    return 0;
}
