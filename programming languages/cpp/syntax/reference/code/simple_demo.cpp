#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {10};
    int &ref_number {number};

    cout << "number = " << number << " | ref_number = " << ref_number << endl;

    ref_number = 11;
    cout << "number = " << number << " | ref_number = " << ref_number << endl;

    return 0;
}
