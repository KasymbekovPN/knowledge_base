#include <iostream>
#include <string>
#include <unordered_map>

int main() {
    std::unordered_map<std::string, int> map {
        {"one", 1}
    };

    std::cout
        << "max_size: "
        << map.max_size()
        << std::endl;

    return 0;
}
