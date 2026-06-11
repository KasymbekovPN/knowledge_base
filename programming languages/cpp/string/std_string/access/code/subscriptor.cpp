#include <iostream>
#include <string>

int main() {
    const std::string str {"Hello"};
    for (auto &item: str) {
        std::cout << item << std::endl;
    }
    
    return 0;
}
