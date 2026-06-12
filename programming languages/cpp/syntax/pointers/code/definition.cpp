#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int* p_int0;
    int *p_int1 {};
    int* p_int2 {nullptr};
    double* p_double {};
    
    cout << "p_int0 <= " << p_int0 << "\t| size <= " << sizeof(p_int0) << endl;
    cout << "p_int1 <= " << p_int1 << "\t| size <= " << sizeof(p_int1) << endl;
    cout << "p_int2 <= " << p_int2 << "\t| size <= " << sizeof(p_int2) << endl;
    cout << "p_double <= " << p_double << "\t| size <= " << sizeof(p_double) << endl;

    return 0;
}
