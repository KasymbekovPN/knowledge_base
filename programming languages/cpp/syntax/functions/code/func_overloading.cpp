#include <iostream>

using std::cout;
using std::endl;

int sum(int, int);
int sum(int, int, int);
double sum(double, double);

int main(int argc, char const *argv[]) {
    const double d0 {3.3};
    const double d1 {4.4};
    const double d2 {5.5};

    cout << "variant 0: <= " << sum((int) d0, (int) d1) << endl;
    cout << "variant 1: <= " << sum(d0, d1, d2) << endl;
    cout << "variant 2: <= " << sum(d0, d1) << endl;

    return 0;
}

int sum(int a0, int a1) {
    return a0 + a1;
}

int sum(int a0, int a1, int a2) {
    return a0 + a1 + a2;
}

double sum(double a0, double a1) {
    return a0 + a1;
}
