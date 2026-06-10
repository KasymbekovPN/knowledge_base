#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {10};
    int b {5};
    int c {a + b};
    int d {c + 10};

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;
    cout << "d <= " << d << endl;

    return 0;
}
