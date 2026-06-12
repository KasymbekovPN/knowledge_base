#include <iostream>

int main(int argc, char const *argv[]){
    signed char signed_char_value{-64};
    unsigned char unsigned_char_value{64};
    short short_value{-88};
    unsigned short unsigned_short_value{88};
    int int_value{-1024};
    unsigned int unsigned_int_value{1024};
    long long_value{-2048};
    unsigned long unsigned_long_value{2048};
    long long long_long_value{-4096};
    unsigned long long unsigned_long_long_value{4096};
    std::cout << "signed_char_value: " << signed_char_value << "\n";
    std::cout << "unsigned_char_value: " << unsigned_char_value << "\n";
    std::cout << "short_value: " << short_value << "\n";
    std::cout << "unsigned_short_value: " << unsigned_short_value << "\n";
    std::cout << "int_value: " << int_value << "\n";
    std::cout << "unsigned_int_value: " << unsigned_int_value << "\n";
    std::cout << "long_value: " << long_value << "\n";
    std::cout << "unsigned_long_value: " << unsigned_long_value << "\n";
    std::cout << "long_long_value: " << long_long_value << "\n";
    std::cout << "unsigned_long_long_value: " << unsigned_long_long_value << "\n";

    return 0;
}
