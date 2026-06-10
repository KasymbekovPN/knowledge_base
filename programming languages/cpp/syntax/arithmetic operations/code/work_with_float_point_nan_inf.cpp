#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    double a {1.5}, b{}, c{}, d{-1.5};
    double ab {a / b};
    double dc {d / c};
    double bc {b / c};

    cout << "a / b => " << ab << endl;
    cout << "d / c => " << dc << endl;
    cout << "b / c => " << bc << endl;
    cout << "b / c + a => " << bc + a << endl;

    return 0;
}
