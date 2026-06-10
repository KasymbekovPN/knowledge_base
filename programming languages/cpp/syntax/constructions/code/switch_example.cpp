#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {1};

    switch (a) {
        case 0:
        case 1: {
            int x{123};
            cout << a << " <> " << x << endl;
            break;
        }
        
        default: {
            int x{456};
            cout << a << " <> " << x << endl;
            break;
        }
    }

    return 0;
}
