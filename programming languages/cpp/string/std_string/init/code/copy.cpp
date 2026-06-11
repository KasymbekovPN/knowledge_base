#include <iostream>
#include <string>

int main() {
    const std::string original = "Hello";
    std::string line {original};
    std::cout
        << "'" << line << "' size: "
        << line.size() << std::endl;

    return 0;
}
