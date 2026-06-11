#include <iostream>
#include <string>
#include <string_view>

int main() {
    const std::string_view sv {"Example from literal"};
    std::cout << "size <= " << sv.size() << std::endl;

    return 0;
}
