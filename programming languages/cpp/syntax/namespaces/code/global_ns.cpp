#include <iostream>

const std::string MESSAGE {"Hello, world !!!"};

void print(const std::string&);

int main(int argc, char const *argv[]) {
    print(MESSAGE);
    ::print(MESSAGE);
    return 0;
}

void print(const std::string& text) {
    std::cout << text << std::endl;
}
