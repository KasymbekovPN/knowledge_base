#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    
    for (int i{}; i < 20; i++) {
        if (i % 2 == 0) {
            continue;
        }
        if (i > 15) {
            break;
        }

        cout << "i <= " << i << endl;
    }

    return 0;
}
