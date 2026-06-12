#include <iostream>

int main(int argc, char const *argv[])
{
    unsigned int unsigned_int_value{1024U};
    long long_value{-2048L};
    unsigned long unsigned_long_value{20248UL};
    long long long_long_value{-4096LL};
    unsigned long long unsigned_long_long_value{4096ULL};

    std::cout << "unsigned_int_value: " << unsigned_int_value << "\n";
    std::cout << "long_value: " << long_value << "\n";
    std::cout << "unsigned_long_value: " << unsigned_long_value << "\n";
    std::cout << "long_long_value: " << long_long_value << "\n";
    std::cout << "unsigned_long_long_value: " << unsigned_long_long_value << "\n";

    return 0;
}
