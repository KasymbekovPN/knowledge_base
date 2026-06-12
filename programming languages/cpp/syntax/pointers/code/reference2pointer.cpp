#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int first {1};
    int second {42};
    int *pointer {};
    int *&pref {pointer};

    pref = &first;
    cout << "*pointer <= " << *pointer << endl;

    *pref = 111;
    cout << "first <= " << first << endl;

    pref = &second;
    cout << "*pointer <= " << *pointer << endl;

    return 0;
}
