#include <iostream>
#include <cstring>

int main() {
    const char src[] = "Hello";
    std::cout << "src <= " << src << std::endl;

    char dest[8];
    strcpy(dest, src);
    std::cout << "dest <= " << dest << std::endl;

    return 0;
}
