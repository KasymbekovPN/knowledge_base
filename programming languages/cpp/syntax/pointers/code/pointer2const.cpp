#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    const int number {42};
    const int* pnumber {&number};

    cout << "addr <= " << pnumber << " | value <= " << *pnumber << endl;

    return 0;
}
