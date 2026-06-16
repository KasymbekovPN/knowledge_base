#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    unsigned int ui0 {25};
    // unsigned int ui1 {-25}; // error

    cout << "ui0 <= " << ui0 << endl;
    // cout << "ui1 <= " << ui1 << endl;

    return 0;
}
