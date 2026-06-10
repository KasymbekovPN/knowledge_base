#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {26};
    int b {5};
    int c {4};

    int ab {a % b};
    int cb {c % b};

    cout << "ab < = " << ab << endl;
    cout << "cb < = " << cb << endl;

    return 0;
}
