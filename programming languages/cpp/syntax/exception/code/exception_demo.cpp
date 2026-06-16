#include <iostream>

int divide(int, int);

int main(int argc, char const *argv[]) {
    int result {divide(100, 0)};
    std::cout << "result <= " << result << std::endl;

    return 0;
}

int divide(int a, int b) {
    return a / b;
}
