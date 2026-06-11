#include <iostream>

int main() {
    const char str0[] = "Hello";
    const char str1[10] = "World";
    const char str2[] = {'!', '!', '!', '\0'};

    std::cout << "str0 <= " << str0 << std::endl;
    std::cout << "str1 <= " << str1 << std::endl;
    std::cout << "str2 <= " << str2 << std::endl;

    return 0;
}
