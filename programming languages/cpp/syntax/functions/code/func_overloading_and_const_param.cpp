#include <iostream>

using std::cout;
using std::endl;

int square(const int*);
int square(int*);

int main(int argc, char const *argv[]){
    const int cnumber {42};
    int number {43};

    cout << "square(&cnumber) <= " << square(&cnumber) << endl;
    cout << "square(&number) <= " << square(&number) << endl;

    return 0;
}

int square(const int* p_number) {
    return *p_number * *p_number;
}

int square(int* p_number) {
    *p_number = *p_number * *p_number;

    return *p_number;
}
