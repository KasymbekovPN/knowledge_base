#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a{};
    a = 2;
    
    int b{}, c{}, d{};
    b = c = d = 42;

    int e{};
    e = 3 + 5;

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;
    cout << "d <= " << d << endl;
    cout << "e <= " << e << endl;

    return 0;
}
