#include <iostream>
#include <cstring>

int main() {
    char src[] = "Hello world!!!";

    char dest0[10];
    errno_t result0 = strcpy_s(dest0, sizeof(dest0), src);
    if (result0 == 0) {
        std::cout << "[0+] " << dest0 << std::endl;
    } else {
        std::cout << "[0-]" << std::endl;
    }

    return 0;
}
