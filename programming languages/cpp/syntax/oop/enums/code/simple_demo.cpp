#include <iostream>

enum class EnumExample {
    Zero,
    First,
    Second
};

enum class Day {
    Monday = 2,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday = 1
};

enum class Operation {
    Add = '+',
    Substract = '-',
    Multiply = '*'
};

int main(int argc, char const *argv[]) {
    std::cout << "EnumnExample::Zero <= " << (int) EnumExample::Zero << std::endl;
    std::cout << "EnumnExample::First <= " << (int) EnumExample::First << std::endl;
    std::cout << "EnumnExample::Second <= " << (int) EnumExample::Second << std::endl << std::endl;
    
    std::cout << "Day::Monday <= " << (int) Day::Monday << std::endl;
    std::cout << "Day::Tuesday <= " << (int) Day::Tuesday << std::endl;
    std::cout << "Day::Wednesday <= " << (int) Day::Wednesday << std::endl;
    std::cout << "Day::Thursday <= " << (int) Day::Thursday << std::endl;
    std::cout << "Day::Friday <= " << (int) Day::Friday << std::endl;
    std::cout << "Day::Saturday <= " << (int) Day::Saturday << std::endl;
    std::cout << "Day::Sunday <= " << (int) Day::Sunday << std::endl << std::endl;

    std::cout << "Operation::Add <= '" << static_cast<char>(Operation::Add) << "'" << std::endl;
    std::cout << "Operation::Substract <= '" << static_cast<char>(Operation::Substract) << "'" << std::endl;
    std::cout << "Operation::Multiply <= '" << static_cast<char>(Operation::Multiply) << "'" << std::endl;

    return 0;
}
