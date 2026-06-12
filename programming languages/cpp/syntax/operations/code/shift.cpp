#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    unsigned int offset {3};
    unsigned int original {7};
    unsigned int result {original << offset};
    cout << original << " << " << offset << " = " << result << endl;

    original = 26;
    result = original >> offset;
    cout << original << " >> " << offset << " = " << result << endl;

    return 0;
}
