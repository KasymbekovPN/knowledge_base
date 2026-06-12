#include <iostream>

using std::cout;
using std::boolalpha;
using std::noboolalpha;
using std::endl;

int main(int argc, char const *argv[]) {
    int a{8};
    int b{11};

    bool r0{a == b};
    bool r1{a != b};
    bool r2{a > b};
    bool r3{a < b};

    cout << "(" << a << " == " << b << ") => " << noboolalpha << r0 <<  ", " << boolalpha << r0 << endl;
    cout << "(" << a << " != " << b << ") => " << noboolalpha << r1 <<  ", " << boolalpha << r1 << endl;
    cout << "(" << a << " > "  << b << ") => " << noboolalpha << r2 <<  ", " << boolalpha << r2 << endl;
    cout << "(" << a << " < "  << b << ") => " << noboolalpha << r3 <<  ", " << boolalpha << r3 << endl;

    return 0;
}
