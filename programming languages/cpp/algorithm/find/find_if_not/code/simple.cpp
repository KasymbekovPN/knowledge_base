#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

bool _check(const int);
void _test_find_if_not(const vector<int>&);

int main() {
    const vector<int> v0 {1, 2, 3, 4, 5};
    const vector<int> v1 {4, 5, 6};
    const vector<int> v2 {2, 4, 6};

    _test_find_if_not(v0);
    _test_find_if_not(v1);
    _test_find_if_not(v2);

    return 0;
}

bool _check(const int value) {
    return value % 2 == 0;
}

void _test_find_if_not(const vector<int>& vec) {
    auto it = find_if_not(vec.begin(), vec.end(), _check);
    if (it != vec.end()) {
        cout
            << "First odd element " << *it
            <<  " at position " << (it - vec.begin())
            << endl;
    } else {
        cout << "No odd numbers found" << endl;
    }
}
