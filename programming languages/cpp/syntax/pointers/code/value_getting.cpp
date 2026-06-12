#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {42};
    int* pnumber {&number};
    int number_copy {*pnumber};

    cout << "number <= " << number << endl;
    cout << "pnumber <= " << *pnumber << endl;
    cout << "number_copy <= " << number_copy << endl;

    *pnumber = 24;

    cout << "number <= " << number << endl;
    cout << "pnumber <= " << *pnumber << endl;
    cout << "number_copy <= " << number_copy << endl;

    return 0;
}
