#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    cout << "SAFETY" << endl;
    short a = 'g';  // char -> short
    int b = 10;
    double c = b;   // int -> double
    float d = 3.4;
    double e = d;   // float -> double
    double f = 35;  // int -> double
    cout << "a <= "  << a << endl;
    cout << "c <= "  << c << endl;
    cout << "e <= "  << e << endl;
    cout << "f <= "  << f << endl;

    cout << "UNSAFETY" << endl;
    unsigned int u0 = -25; 
    unsigned short u1 = -3500;
    cout << "u0 <= " << u0 << endl;
    cout << "u1 <= " << u1 << endl;

    // ERROR
    // unsigned int e0 {-25};
    // unsigned short e1 {-3500};

    return 0;
}
