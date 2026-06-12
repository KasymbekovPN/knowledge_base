#include <iostream>

using std::cout;
using std::endl;

void square(int);

int main(int argc, char const *argv[]) {
    int number {42};
    square(number);

    return 0;
}

void square(const int number) {
    // number = 2; // Error
    int n = number;
    cout << "result <= " << n * n << endl;
}
