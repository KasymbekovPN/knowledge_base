#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    for (size_t i {}; i < argc; i++) {
        cout << "[" << i << "] " << argv[i] << endl;
    }
    cout << "Done." << endl;
    
    return 0;
}
