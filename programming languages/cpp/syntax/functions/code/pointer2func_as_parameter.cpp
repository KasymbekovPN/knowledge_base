#include <iostream>

using std::cout;
using std::endl;

int add(int, int);
int sub(int, int);
int operation(int (*)(int, int), int, int);

int main(int argc, char const *argv[]) {
    const int x {42};
    const int y {73};

    cout << "ADD result <= " << operation(add, x, y) << endl;
    cout << "SUB result <= " << operation(sub, x, y) << endl;

    return 0;
}

int add(int x, int y) {
    return x + y;
}

int sub(int x, int y) {
    return x - y;
}

int operation(int (*op)(int, int), int x, int y) {
    return op(x, y);
}
