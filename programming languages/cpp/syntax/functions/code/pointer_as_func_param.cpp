#include <iostream>

using std::cout;
using std::endl;

void func(int*, bool swap);

int main(int argc, char const *argv[]) {
    int number {42};
    int* pnumber {&number};
    
    func(pnumber, false);
    cout << "*pnumber <= " << *pnumber << endl;
    
    func(pnumber, true);
    cout << "*pnumber <= " << *pnumber << endl;

    return 0;
}

void func(int* ptr, bool swap) {
    cout << ptr << " | " << *ptr << endl; 
    (*ptr)++;
    if (swap) {
        int new_value {111};
        ptr = &new_value;
    }
    cout << ptr << " | " << *ptr << endl; 
}
