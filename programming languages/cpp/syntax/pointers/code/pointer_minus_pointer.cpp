#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int numa {7};
    int numb {8};
    int* pnuma {&numa};
    int* pnumb {&numb};

    auto sub {pnuma - pnumb};
    cout << pnuma << " - " << pnumb << " = " << sub << endl;

    return 0;
}
