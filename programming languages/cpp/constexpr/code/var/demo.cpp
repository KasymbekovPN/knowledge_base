#include <iostream>
#include <array>

using namespace std;

constexpr int SIZE = 10;
constexpr double PI = 3.1415926535;

int main() {
    array<double, SIZE> arr;
    arr[3] = PI;
    cout << "arr[3] => " << arr[3] << endl;

    return 0;
}
