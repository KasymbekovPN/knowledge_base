#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {42};
    int* pnumber {&number};

    cout << "number <= " << number
         << " | *number <= " << *pnumber
         << " | number <= " << pnumber << endl;

    *pnumber = 12;

    cout << "number <= " << number
         << " | *number <= " << *pnumber
         << " | number <= " << pnumber << endl;

    return 0;
}
