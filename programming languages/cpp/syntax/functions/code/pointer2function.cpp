#include <iostream>

void hello(std::string);
void goodbye(std::string);

int main(int argc, char const *argv[]) {
    void (*message)(std::string);

    message = hello;
    message("H");

    message = goodbye;
    message("G");

    return 0;
}

void hello(std::string level) {
    std::cout << "[hello] " << level << std::endl;
}

void goodbye(std::string level) {
    std::cout << "[goodbye] " << level << std::endl;
}
