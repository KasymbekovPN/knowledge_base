#include <iostream>
#include <array>

int main() {
    auto array = std::to_array({111, 222, 333});

    for (auto &&item:  array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
