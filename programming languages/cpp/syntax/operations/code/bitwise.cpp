#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a = 5 | 2; // 101 | 010 = 111 (7)
    int b = 6 & 2; // 110 & 010 =  10 (2)
    int c = 5 ^ 2; // 101 ^ 010 = 111 (7)
    int d = ~9;    // -10

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;
    cout << "d <= " << d << endl;

    return 0;
}
