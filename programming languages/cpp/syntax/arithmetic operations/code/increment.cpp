#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {7};
    int b {++a};
    int c {8};
    int d {c++};

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;
    cout << "d <= " << d << endl;

    return 0;
}
