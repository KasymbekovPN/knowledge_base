#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {5};
    int b {10};
    double c {123.4};
    double d {56.7};

    int ab {a / b};
    double bc {b / c};
    double cd {c / d};

    cout << "ab <= " << ab << endl;
    cout << "bc <= " << bc << endl;
    cout << "cd <= " << cd << endl;
    
    return 0;
}
