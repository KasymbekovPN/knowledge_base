#include <iostream>

enum class Day {Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday};

using enum Day;

int main(int argc, char const *argv[]) {
    Day today {Friday};
    std::cout << static_cast<int>(today) << std::endl;
    
    return 0;
}
