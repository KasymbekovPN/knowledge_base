#include <iostream>
#include <string>
#include <string_view>

int main() {
    const std::string_view sv {"Example from literal"};
    std::cout << "data result <= " << sv.data() << std::endl;

    return 0;
}
