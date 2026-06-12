#include <iostream>

using std::cout;
using std::endl;

int n {5};

void print();

int main(int argc, char const *argv[]) {
    print();
    n++;

    cout << "main: n <= " << n << endl;

    return 0;
}

void print() {
    n++;
    cout << "print: n <= " << n << endl;
}
