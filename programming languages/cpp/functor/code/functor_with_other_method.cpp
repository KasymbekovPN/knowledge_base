#include <iostream>

using std::cout;
using std::endl;

class Adder {

public:
    int operator()(int a, int b) const noexcept{
        return a + b; 
    }

    void print_help() const {
        cout << "Some help" << endl;
    }
};

int main() {
    Adder adder;
    cout << adder(42, 12) << endl;
    adder.print_help();

    return 0;
}
