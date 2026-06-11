#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

void _print_vector(const vector<int>&);

int main() {
    const vector<int> SOURCE {1, 2, 3, 4, 5};

    vector<int> dst;
    copy_if(
        SOURCE.begin(),
        SOURCE.end(),
        back_inserter(dst),
        [](int x) { return x % 2 == 0; }
    );
    _print_vector(dst);

    return 0;
}

void _print_vector(const vector<int>& v) {
    cout << "{";
    for (auto &item: v) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
