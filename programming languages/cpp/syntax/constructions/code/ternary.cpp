#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a{5};
    int b{3};

    if (a > b) {
        cout << "[if] a > b" << endl;
    } else {
        cout << "[if] a <= b" << endl;
    }
    
    cout << (a > b ? "[T] a > b" : "[T] a <= b") << endl;
    
    return 0;
}
