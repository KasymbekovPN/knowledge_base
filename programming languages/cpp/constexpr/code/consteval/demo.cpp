#include <iostream>

using namespace std;

consteval int sqr(int _x) { return _x*_x; }

int main() {
    constexpr int a = sqr(5);
    cout << a << endl;

    int x = 42;
    // int b = sqr(x); // Error

    return 0;
}
