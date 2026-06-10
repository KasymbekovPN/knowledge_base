#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int i{5};

    do {
        cout << "i <= " << i-- << endl;
    } while (i > 0);

    return 0;
}
