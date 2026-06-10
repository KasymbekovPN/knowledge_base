#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a{8};
    int b{};

    if (a) {
        cout << "a likes true" << endl;
    } else {
        cout << "a likes false" << endl;
    }

    if (b) {
        cout << "b likes true" << endl;
    }
    else {
        cout << "b likes false" << endl;
    }

    return 0;
}
