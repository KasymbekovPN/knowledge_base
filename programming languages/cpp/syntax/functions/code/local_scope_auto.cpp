#include <iostream>

using std::cout;
using std::endl;

void print();

int main(int argc, char const *argv[]) {
    print();

    int m {7};
    cout << "m <= " << m << endl;
    cout << "n <= " << n << endl; // Error

    return 0;
}

void print() {
    int n {5};
    cout << "n <= " << n << endl;
}
