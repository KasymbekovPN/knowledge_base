#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {42};
    int* pnumber {&number};

    cout << "  number <= " << number << endl;
    cout << " pnumber <= " << pnumber << endl;
    cout << "*pnumber <= " << *pnumber << endl;

    return 0;
}
