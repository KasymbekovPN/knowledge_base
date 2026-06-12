#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {42};
    int *pointer {&number};

    cout << "pointer <= " << pointer << endl;

    return 0;
}
