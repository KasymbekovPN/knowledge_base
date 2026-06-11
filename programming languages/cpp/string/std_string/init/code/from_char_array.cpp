#include <iostream>
#include <string>

int main() {
    const char original[] = "Hello, world!!!";
    std::string line {original};
    std::cout
        << "'" << line << "' size: "
        << line.size() << std::endl;

    return 0;
}
