#include <iostream>
#include <span>

int main(int argc, char const *argv[]) {
    std::span<int> s;
    std::cout << "size: " << s.size() << std::endl;

    return 0;
}
