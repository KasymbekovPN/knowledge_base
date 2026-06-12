#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int numbers[] {0, 1, 2, 3, 4};
    int* pointer {numbers};
    int number2 {*(pointer + 2)};
    cout << "number2 <= " << number2 << endl;

    int* pointer2 {&numbers[2]};
    cout << "*pointer2 <= " << *pointer2 << endl;

    for (int* ptr{numbers}; ptr <= &numbers[4]; ptr++) {
        cout << "addr <= " << ptr << " | value <= " << *ptr << endl;
    }
    
    return 0;
}
