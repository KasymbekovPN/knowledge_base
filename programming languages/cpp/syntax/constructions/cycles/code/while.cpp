#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int i{};

    while (++i <= 3) {
        cout << "i <= " << i << endl;
    }
    cout << "Done" << endl;
    
    return 0;
}
