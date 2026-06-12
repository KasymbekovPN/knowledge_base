#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    char hello[] {"hello"};
    char* phello {hello};

    cout << "phello  <= " << phello << endl;
    cout << "address <= " << (void*)phello << endl;

    return 0;
}
