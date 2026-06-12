#include <iostream>

using std::cout;
using std::endl;

void func(const int* const);

int main(int argc, char const *argv[]) {
    int number {42};
    func(&number);

    return 0;
}

void func(const int* const ptr) {
    int new_value {146};
    // ptr = &new_value; // Error

    cout << "new_value <= " << new_value << endl;
    cout << "*ptr <= " << *ptr << endl;
}
