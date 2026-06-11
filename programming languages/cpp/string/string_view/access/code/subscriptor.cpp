#include <iostream>
#include <string>
#include <string_view>

int main() {
    const std::string_view sv {"ABC"};
    for (size_t i {}; i < sv.size(); i++) {
        std::cout << sv[i];
    }
    std::cout << std::endl;
    
    return 0;
}
