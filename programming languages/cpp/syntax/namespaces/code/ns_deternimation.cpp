#include <iostream>

namespace hello {
    const std::string MESSAGE {"hello"};
    void print(const std::string&);
    void print_default();
}

int main(int argc, char const *argv[]) {
    hello::print("test text");
    hello::print_default();

    return 0;
}

void hello::print(const std::string& text) {
    std::cout << text << std::endl;
}

void hello::print_default() {
    print(MESSAGE);
}
