#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    float a {1.23e-4};
    float b {3.65e+6};
    float ab {a + b};

    cout << a << " + " << b << " = " << ab << endl;

    return 0;
}
