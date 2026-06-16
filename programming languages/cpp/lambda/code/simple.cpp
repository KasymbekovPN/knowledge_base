#include <iostream>

int main() {
    auto f = [](){std::cout << "Hello" << std::endl;};
    f();

    return 0;
}
