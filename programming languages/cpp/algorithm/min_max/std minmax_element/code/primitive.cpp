#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    const vector<int> NUMBERS {5, 2, 8, 1, 9, 3};

    auto&& [min_it0, max_it0] = minmax_element(NUMBERS.begin(), NUMBERS.end());
    cout << "1) min: " << *min_it0 << endl;
    cout << "1) max: " << *max_it0 << endl;

    auto&& [min_it1, max_it1] = minmax_element(
        NUMBERS.begin(),
        NUMBERS.end(),
        [](int a, int b) { return a > b; }
    );
    cout << "2) min: " << *min_it1 << endl;
    cout << "2) max: " << *max_it1 << endl;

    return 0;
}
