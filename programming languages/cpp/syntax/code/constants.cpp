#include <iostream>

int main() {
    const int i0 = 123;
    const int i1 {456};
    const int i2 {i0};
    const int i3 {789};
    // const int i4;

    std::cout << "i0 <= " << i0 << "\n";
    std::cout << "i1 <= " << i1 << "\n";
    std::cout << "i2 <= " << i2 << "\n";
    std::cout << "i3 <= " << i3 << "\n";

    // i3 = 0;

    return 0;
}
