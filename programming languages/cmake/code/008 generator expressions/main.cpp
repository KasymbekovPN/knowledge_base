#include <iostream>

int main() {
    std::cout << "Hello World!\n";

#ifdef ENABLE_ASSERTS
    std::cout << "ENABLE_ASSERTS.\n";
#endif

#ifdef NDEBUG
    std::cout << "NDEBUG.\n";
#endif

    return 0;
}