#include <iostream>

enum Day {Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday};

int main(int argc, char const *argv[]) {
    Day today = Tuesday;
    std::cout << today << std::endl;

    return 0;
}
