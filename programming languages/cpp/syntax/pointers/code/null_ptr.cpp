#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int* p0 {};
    int* p1 {nullptr};

    cout << "p0 <= " << p0 << endl;
    cout << "p1 <= " << p1 << endl;

    return 0;
}
