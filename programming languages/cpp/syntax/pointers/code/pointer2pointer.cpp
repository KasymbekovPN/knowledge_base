#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int first {42};
    int second {1};

    int* p0 {&first};
    int* p1 {&second};

    cout << "*p0 <= " << *p0 << "\t| p0 <= " << p0 << endl;
    cout << "*p1 <= " << *p1 << "\t| p1 <= " << p1 << endl;

    p0 = p1;
    *p0 = 123;
    cout << "*p0 <= " << *p0 << "\t| p0 <= " << p0 << endl;
    cout << "*p1 <= " << *p1 << "\t| p1 <= " << p1 << endl;

    return 0;
}
