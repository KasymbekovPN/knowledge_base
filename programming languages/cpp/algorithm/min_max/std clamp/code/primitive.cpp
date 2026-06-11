#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

int main() {
    const vector<int> NUMBERS {-12, -11, -10, -9, 0, 9, 10, 11, 12};
    for (const auto &number: NUMBERS) {
        cout 
            << endl
            << "Original: " << number << endl
            << "Clamped[0, 10]: " << clamp(number, 0, 10) << endl
            << "Clamped[-10, 10, abs]: "
            <<  clamp(
                number,
                -10,
                10,
                [](int a, int b) { return abs(a) < abs(b); }
            )
            << endl;
    }

    return 0;
}
