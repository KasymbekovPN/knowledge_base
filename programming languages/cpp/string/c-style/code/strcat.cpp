#include <iostream>
#include <cstring>

int main() {
    char dest[16] = "Hello, ";
    strcat(dest, "world");
    std::cout << "dest => " << dest << std::endl;

    return 0;
}
