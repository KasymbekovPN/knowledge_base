#include <iostream>

void sum(auto, auto);

int main(int argc, char const *argv[]) {
    sum(11, 12);
    sum(12.3, 45.6);

    return 0;
}

void sum(auto a0, auto a1) {
    auto result = a0 + a1;
    std::cout << "result <= " << result << std::endl; 
}
