#include <iostream>

int main(int argc, char const *argv[]) {
    // char8_t ch8 {u8'A'};
    char16_t ch16 {u'B'};
    char32_t ch32 {U'C'};

    // std::wcout << "ch8 <= " << ch8 << "\n";
    std::wcout << "ch16 <= " << ch16 << "\n";
    std::wcout << "ch32 <= " << ch32 << "\n";

    return 0;
}
