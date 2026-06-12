#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number0 {42};
    int number1 {1};

    int *pnumber0 {&number0};
    int *pnumber1 {&number1};

    bool comparison_result = pnumber0 > pnumber1;
    if (comparison_result) {
        cout << "(" << pnumber0 << ") > " << "(" << pnumber1 << ")";
    } else {
        cout << "(" << pnumber0 << ") <= " << "(" << pnumber1 << ")";
    }

    return 0;
}
