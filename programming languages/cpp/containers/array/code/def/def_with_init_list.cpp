#include <iostream>
#include <array>

int main() {
    std::array<int, 5> array {0, 1, 2, 3, 4};
    for (auto &&item:  array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
