#include <iostream>

int main(int argc, char const *argv[]){
    int hex_int_value{0x1A};
    int oct_int_value{034};
    int bin_int_value{0b1010};

    std::cout << "hex_int_value: " << hex_int_value << "\n";
    std::cout << "oct_int_value: " << oct_int_value << "\n";
    std::cout << "bin_int_value: " << bin_int_value << "\n";

    return 0;
}
