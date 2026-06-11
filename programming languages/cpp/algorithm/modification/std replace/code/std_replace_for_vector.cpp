#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _print_vector(const vector<int>&);

int main() {
    vector<int> v {10, 20, 30, 20, 50};

    replace(v.begin(), v.end(), 20, 42);
    _print_vector(v);

    return 0;
}

void _print_vector(const vector<int>& v) {
    cout << "{ ";
    for (auto &i: v) {
        cout << i << " ";
    }
    cout << "}" << endl;
}
