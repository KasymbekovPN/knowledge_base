#include <format>
#include <iostream>
#include <format>

namespace {

    struct Normal {
        // 1 байт + 3 padding
        char a;
        // 4 байта
        int b;
        // 1 байт + 3 padding
        char c;
    };

    struct __attribute__((__packed__)) Packed {
        // 1 байт
        char a;
        // 4 байта
        int b;
        // 1 байт
        char c;
    };

}

int main() {
    std::cout << std::format("Normal: {}\n", sizeof(Normal));
    std::cout << std::format("Packed: {}\n", sizeof(Packed));

    return 0;
}
