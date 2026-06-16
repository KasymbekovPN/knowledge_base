#include <iostream>
#include <exception>

double divide(double, double);

int main(int argc, char const *argv[]){
    try {
        double result {divide(100.0, 0.0)};
        std::cout << "result <= " << result << std::endl;
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    std::cout << "DONE!" << std::endl;

    return 0;
}

double divide(double a, double b) {
    if (!b) {
        throw std::exception();
    }
    return a / b;
}
