#include <iostream>
#include <cstring>

int main() {
    const char str0[] = "Hello";
    const char str1[10] = "World";
    const char str2[] = {'!', '!', '!', '\0'};

    std::cout << "len of str0 <= " << strlen(str0) << std::endl;
    std::cout << "len of str1 <= " << strlen(str1) << std::endl;
    std::cout << "len of str2 <= " << strlen(str2) << std::endl;

    return 0;
}
