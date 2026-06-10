#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {10};
    int b {7};
    int c {a * b};
    int d {-3 * c};

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;
    cout << "d <= " << d << endl;

    return 0;
}
