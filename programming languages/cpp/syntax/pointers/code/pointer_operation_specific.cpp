#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number0 {10};
    int* pnumber0 {&number0};
    int result0 {*pnumber0 + 1};
    cout << "result0 <= " << result0 << endl;

    int number1 {20};
    int* pnumber1 {&number1};
    int result1 {++*pnumber1};
    cout << "*pnumber1 <= " << *pnumber1 << endl;
    cout << "result1 <= " << result1 << endl;

    int number2 {30};
    int* pnumber2 {&number2};
    int result2 {*pnumber2++};
    cout << "*pnumber2 <= " << *pnumber2 << endl;
    cout << "result2 <= " << result2 << endl;

    return 0;
}
