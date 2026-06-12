#include <iostream>

using std::cout;
using std::endl;

int& max(int&, int&);

int main(int argc, char const *argv[]) {
    int n{4};
    int m{5};

    int result = max(n, m);
    cout << "max(" << n << ", " << m << ") <= " << result  << endl;

    return 0;
}

int& max(int& n, int& m) {
    return n > m ? n : m;
}
