#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int other {1};
    int number {42};
    const int* const pnumber {&number};
    cout << "pnumber <= " << *pnumber << endl;

    // *pnumber = 123; // Error
    // pnumber = &other; // Error

    return 0;
}
