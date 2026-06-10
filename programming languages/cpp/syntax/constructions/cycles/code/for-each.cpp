#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    for (int n : {1, 2, 3}) {
        cout << "n <= " << n << endl;
    }

    for (char ch: "Hello") {
        cout << "ch <= '" << ch << "'" << endl;
    }

    return 0;
}
