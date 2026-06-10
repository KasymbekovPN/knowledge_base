#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a{5};
    int b{3};

    if (int c{a-b}; a > b) {
        cout << "true <> " << c << endl;
    } else {
        cout << "false <> " << c << endl;
    }

    if (int rem {a % b}; rem == 0) {
        cout << "true <> " << rem << endl;
    } else {
        cout << "false <> " << rem << endl;
    }

    return 0;
}
