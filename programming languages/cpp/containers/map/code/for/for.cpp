#include <iostream>
#include <map>

int main() {
    const std::map<std::string, int> map {
        {"one", 1},
        {"two", 2}
    };

    for (auto &i: map) {
        std::cout
            << "[" << i.first
            << ", " << i.second
            << "]" << std::endl;
    }
    
    for (auto &[key, value]: map) {
        std::cout
            << "[" << key
            << ", " << value
            << "]" << std::endl;
    }
    
    return 0;
}
