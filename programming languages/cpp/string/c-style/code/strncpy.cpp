#include <iostream>
#include <cstring>

int main() {
    const char src[] = "0123456789";
    
    char dest_long[16];
    strncpy(dest_long, src, sizeof(dest_long) - 1);
    dest_long[sizeof(dest_long) - 1] = '\0';
    std::cout << "dest_long <= " << dest_long << std::endl;

    char dest_short[5];
    strncpy(dest_short, src, sizeof(dest_short) - 1);
    dest_short[sizeof(dest_short) - 1] = '\0';
    std::cout << "dest_short <= " << dest_short << std::endl;

    return 0;
}
