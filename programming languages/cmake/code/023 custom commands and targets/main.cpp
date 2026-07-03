#include "version.h"
#include <iostream>

int main() {
    std::cout << "Build: " << BUILD_TYPE << '\n';
    std::cout << "Build: " << BUILD_DATE << '\n';
}
