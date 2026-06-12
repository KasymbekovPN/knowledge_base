#include <iostream>

using std::cout;
using std::endl;

void square(int);

int main(int argc, char const *argv[]) {
    int number {42};
    cout << "Before <= " << number << endl;

    square(number);
    cout << "After <= " << number << endl;

    return 0;
}

void square(int n) {
    n = n*n;
    cout << "[square] n <= " << n << endl;
}
