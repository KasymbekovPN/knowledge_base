#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _print_vector(vector<int>&);

int main() {
    const vector<int> SRC_A = {1, 2, 3};
    const vector<int> SRC_B = {10, 20, 30};

    vector<int> dst;
    transform(
        SRC_A.begin(),
        SRC_A.end(),
        SRC_B.begin(),
        back_inserter(dst),
        [](int x, int y) { return x + y; }
    );
    _print_vector(dst);

    return 0;
}


void _print_vector(vector<int>& vector) {
    cout << "{";
    for (auto &&i: vector) {
        cout << i << " ";
    }
    cout << "}" << endl;
}