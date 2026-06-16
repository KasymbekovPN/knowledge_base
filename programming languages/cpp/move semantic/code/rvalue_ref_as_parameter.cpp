#include <iostream>

void _print(int&& value) {
    std::cout << "rvalue: " << value << std::endl; 
}

void _print(int& value) {
    std::cout << "lvalue: " << value << std::endl; 
}

int main() {
    int a {42};
    _print(a);
    _print(12);
    _print(std::move(a));

    return 0;
}
