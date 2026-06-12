#include <iostream>

int main(int argc, char const *argv[]){
    double num0{12.34};
    double num1{1};
    double num2{1.0};
    float num3{23.45F};
    double num4{34.56L};
    double num5{5e3};
    double num6{2.5e-3};

    std::cout << "num0 :" << num0 << "\n";
    std::cout << "num1 :" << num1 << "\n";
    std::cout << "num2 :" << num2 << "\n";
    std::cout << "num3 :" << num3 << "\n";
    std::cout << "num4 :" << num4 << "\n";
    std::cout << "num5 :" << num5 << "\n";
    std::cout << "num6 :" << num6 << "\n";

    return 0;
}
