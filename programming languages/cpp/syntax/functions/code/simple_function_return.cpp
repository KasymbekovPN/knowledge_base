#include <iostream>

using std::cout;
using std::endl;

int calculate(const int, const int, const char);

int main(int argc, char const *argv[]) {
    const int result0 = calculate(1, 2, '+');
    const int result1 = calculate(1, 2, '-');

    cout << "result0 <= " << result0 << endl;
    cout << "result1 <= " << result1 << endl;

    return 0;
}

int calculate(const int a, const int b, const char op) {
    const int DEFAULT_RESULT = 0;
    switch (op)
    {
        case '+':
            return a + b;
        
        case '-':
            return a - b;

        default:
            return DEFAULT_RESULT;
    }
}
