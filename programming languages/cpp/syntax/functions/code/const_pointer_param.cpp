#include <iostream>

using std::cout;
using std::endl;

void func(const int*);
void func_with_swap(const int*);
void func_with_error(const int*);

int main(int argc, char const *argv[]) {
    int number {42};
    const int cnumber {142};

    int* pnumber {&number};
    const int* pcnumber {&cnumber};

    func(pnumber);
    func(pcnumber);

    func_with_swap(pnumber);
    func_with_swap(pcnumber);
    cout << "*pnumber <= " << *pnumber << endl;
    cout << "*pcnumber <= " << *pcnumber << endl;

    // func_with_error(pnumber);

    return 0;
}

void func(const int* ptr) {
    cout << "[func] " << *ptr << endl;
}

void func_with_swap(const int* ptr) {
    int new_value = 123;
    ptr = &new_value;
    cout << "[func_with_swap] " << *ptr << endl;
}

// void func_with_error(const int* ptr) {
//     *ptr = 456; // Error
// }