#include <iostream>

using std::cout;
using std::endl;
using std::boolalpha;

int main(int argc, char const *argv[]) {
    int a{5};
    int b{8};

    bool r0 = a == 5 && b > 8;
    bool r1 = a == 5 || b > 8;
    bool r2 = a == 5 ^ b > 8;

    cout << boolalpha
         << a << " == 5 && "  << b << " > 8" << " => " << r0 << endl
         << a << " == 5 || "  << b << " > 8" << " => " << r1 << endl
         << a << " == 5 ^  "  << b << " > 8" << " => " << r2 << endl;

    return 0;
}
