#include <iostream>
#include <vector>
#include <numeric>

using namespace std;

int main() {
    const vector<int> FIRST {1, 2, 3};
    const vector<int> SECOND {4, 5, 6};

    cout
        << "(1*4) + (2*5) + (3*6) = 4 + 10 + 18 = "
        << inner_product(
            FIRST.begin(), FIRST.end(),
            SECOND.begin(), 0
        )
        << endl;
    cout
        << "(1+4) * (2+5) * (3+6) = 5 * 7 * 9 = "
        << inner_product(
            FIRST.begin(), FIRST.end(),
            SECOND.begin(), 1,
            multiplies<int>(),
            plus<int>()
        )
        << endl;

    return 0;
}
