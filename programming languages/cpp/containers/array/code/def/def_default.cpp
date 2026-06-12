#include <iostream>
#include <array>

int main() {
    std::array<int, 5> array {};
    for (auto &&item:  array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
