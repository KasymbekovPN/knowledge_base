#include <iostream>
#include <cstring>

int main() {
    char dest[11] = "01234";
    strncat(dest, "56789abc", 2);
    std::cout << "dest => " << dest << std::endl;

    return 0;
}
