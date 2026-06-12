#include <iostream>
#include <map>

int main() {
    std::map<std::string, int> emptym;
    std::cout
        << "Is it empty? "
        << std::boolalpha
        << emptym.empty()
        << std::noboolalpha
        << std::endl;

    return 0;
}
