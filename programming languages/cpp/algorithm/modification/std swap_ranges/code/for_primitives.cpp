#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    vector<int> src {10, 20, 30};
    vector<int> dst {1, 2, 3};
    print_vector(src);
    print_vector(dst);

    swap_ranges(src.begin(), src.end(), dst.begin());
    print_vector(dst);

    return 0;
}

void print_vector(const vector<int>& container) {
    cout << "{";
    for (auto &&item : container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
