#include <iostream>

int main(int argc, char const *argv[]) {
    bool is_alive {true};
    bool is_dead {false};
    bool is_default_value {};

    std::cout << "is alive <= " << is_alive << "\n";
    std::cout << "is dead <= " << is_dead << "\n";
    std::cout << "is default value <= " << is_default_value << "\n";

    return 0;
}
