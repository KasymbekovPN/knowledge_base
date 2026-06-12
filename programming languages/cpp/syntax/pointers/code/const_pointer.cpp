#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {42};
    int other {1};
    int* const pnumber {&number};

    cout << "*pnumber <= " << *pnumber << endl;

    (*pnumber)++;
    cout << "*pnumber <= " << *pnumber << endl;

    // pnumber = &other;

    return 0;
}
