#include <iostream>

using std::cout;
using std::endl;

void square(const int&);

int main(int argc, char const *argv[]) {
    int number {12};
    square(number);

    return 0;
}

void square(const int& number) {
    // number = 1; // Error
    cout << "number^2 <= " << number * number << endl;
}
