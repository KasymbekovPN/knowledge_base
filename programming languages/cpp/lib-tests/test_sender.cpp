#include <iostream>
#include <version>

int main() {
#if defined(__cpp_lib_senders)
    std::cout << "+\n";
#else
    std::cout << "-\n";
#endif

    return 0;
}
