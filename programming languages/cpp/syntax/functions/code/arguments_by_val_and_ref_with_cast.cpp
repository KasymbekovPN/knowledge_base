#include <iostream>

using std::cout;
using std::endl;

void print_val(int);
void print_ref(int&);

int main(int argc, char const *argv[]) {
    double number {3.14159};
    
    print_val(number);
    print_ref(number); // Error

    return 0;
}

void print_val(int n){
    cout << "[print_val] n <= " << n << endl;
}

void print_ref(int& n) {
    cout << "[print_ref] n <= " << n << endl;
}
