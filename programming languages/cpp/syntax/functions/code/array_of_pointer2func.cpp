#include <iostream>

using std::cout;
using std::endl;

int add(const int*, const int*);
int substract(const int*, const int*);
int multiply(const int*, const int*);

int main(int argc, char const *argv[]) {
    const int first {10};
    const int second {5};

    const size_t SIZE = 3;
    const std::string names[SIZE] {"add", "sub", "multi"};
    int (*operations[SIZE])(const int*, const int*) = {add, substract, multiply};

    for (size_t i {0}; i < SIZE; i++) {
        cout << names[i] << " => " << operations[i](&first, &second) << endl;
    }

    return 0;
}

int add(const int* p0, const int* p1) {
    return *p0 + *p1;
}

int substract(const int* p0, const int* p1) {
    return *p0 - *p1;
}

int multiply(const int* p0, const int* p1) {
    return *p0 * *p1;
}
