#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    double number {14.6};
    double* pdouble {&number};

    cout << "pdouble <= " << pdouble << endl;
    pdouble += 2;
    cout << "pdouble <= " << pdouble << endl;

    short sh {5};
    short* psh {&sh};

    cout << "psh <= " << psh << endl;
    psh -= 3;
    cout << "psh <= " << psh << endl;

    return 0;
}
