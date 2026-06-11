#include <iostream>
#include <string>

int main() {
    std::string line(7, 'x');
    std::cout
        << "'" << line << "' size: "
        << line.size() << std::endl;

    return 0;
}
