#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

void _print_vector(const vector<int>&);

int main() {
    const vector<int> SOURCE {1, 2, 3, 2, 5};

    vector<int> dst;
    replace_copy(SOURCE.begin(), SOURCE.end(), back_inserter(dst), 2, 42);
    _print_vector(dst);
    
    return 0;
}


void _print_vector(const vector<int>& vector) {
    cout << "{";
    for (auto &i: vector) {
        cout << i << " ";
    }
    cout << "}" << endl;
}
