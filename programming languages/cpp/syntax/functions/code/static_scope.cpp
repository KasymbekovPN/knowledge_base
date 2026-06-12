#include <iostream>

using std::cout;
using std::endl;

void print();

int main(int argc, char const *argv[]) {
    print();
    print();
    print();

    return 0;
}

void print() {
    static int k {10};
    int n {1};

    cout << "n <= " << n << " | k <= " << k << endl;
    n++;
    k++;
}
